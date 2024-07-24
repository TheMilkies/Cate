#include "class.h"
#include "system_functions.h"
#include "common.h"
#include "cmd_args.h"
#include "error.h"
#include <ctype.h>

static void classes_free() {
    for (size_t i = 0; i < ctx.classes.size; i++) {
        CateClass* c = &ctx.classes.data[i];
        free(c->includes.data);
        free(c->defines.data);
        free(c->files.data);
        free(c->objects.data);
        free(c->command_template.data);
    }
    da_free(ctx.classes);
}

void context_reset() {
    classes_free();
    ctx.loaded_files.size = 0;
    st_reset(&ctx.st);
}

void context_free() {
    classes_free();
    da_free(ctx.loaded_files);
    st_free(&ctx.st);
}

void save_string(string_view* s, SavedStringIndexes* arr) {
    size_t index = st_save_sv(&ctx.st, s);
    da_append((*arr), index);
}

static void prepare_objects(CateClass* c) {
    todo("preparing objects");
}

typedef struct {
    SavedStringIndexes object_files;
    SavedStringIndexes flags;
    STIndex standard;
    STIndex out_name;
} Prepared;

static STIndex prepare_out_name(CateClass* c) {
    if(c->out_name.length)
        return st_save_sv(&ctx.st, &c->out_name);
    
    STIndex saved = 0;
    if(c->name.length + 1 >= PATH_MAX) {
        cate_error("object \""sv_fmt"\"'s name is too long!",
            sv_p(c->out_name));
    }

    const string_view* ext = 0;
    struct CateFullPath f = {0};
    switch (c->kind) {
    case CLASS_PROJECT: {
        ext = &cate_target->executable_ending;
        if(c->name.length + ext->length + 1 >= PATH_MAX) {
            cate_error("object \""sv_fmt"\"'s out name is too long!",
                sv_p(c->out_name));
        }
        strncpy(f.x, c->name.text, c->name.length);
        strncat(f.x, ext->text, ext->length);
    }   break;

    case CLASS_LIBRARY: {
        ext = (c->as.lib.kind == LIBRARY_STATIC)
            ? &cate_target->static_ending
            : &cate_target->dynamic_ending;

        //7 for "out/lib"
        if(c->name.length+7+ext->length+1 >= PATH_MAX) {
            cate_error("object \""sv_fmt"\"'s out path is too long!",
            sv_p(c->out_name));
        }

        strncpy(f.x, "out/lib", 8);
        strncat(f.x, c->name.text, c->name.length);
        strncat(f.x, ext->text, ext->length);
        //maybe null terminate?
    }   break;
    
    default:
        cate_error("not project or library, wtf");
        break;
    }
    return st_save_s(&ctx.st, f.x);
}

static STIndex prepare_std(CateClass* c) {
    if(!c->standard.length) return 0;
    if(c->standard.length >= 64)
        cate_error("standard is too long?");

    struct CateFullPath flag = {.x = "-std="};
    strncat(flag.x, c->standard.text, c->standard.length+1);
    return st_save_s(&ctx.st, flag.x);
}

static void create_directories(CateClass* c) {
    if(!cate_sys_mkdir(c->build_dir.text)) {
        cate_error("can't create build folder \"%s\"",
            c->out_name.text);
    }

    size_t index = sv_find(&c->out_name, 0, '/');
    if(index == SV_NOT_FOUND) return;

    char backup = c->out_name.text[index];
    c->out_name.text[index] = 0;

    if(!cate_sys_mkdir(c->out_name.text)) {
        cate_error("can't create build folder \"%s\"",
            c->out_name.text);
    }

    c->out_name.text[index] = backup;
}

static STIndex objectify_file(string_view* dir, string_view* file) {
    struct CateFullPath path = {0};
    if(dir->length + file->length >= PATH_MAX) {
        cate_error("object file length will be too long for \""sv_fmt"\"",
            svptr_p(file));
    }

    //add directory
    size_t length = dir->length;
    strncpy(path.x, dir->text, dir->length);
    if(dir->text[dir->length-1] != '/')
        path.x[length++] = '/';

    size_t last_dot = 0;
    for (size_t i = 0; i < file->length; ++i) {
        switch (file->text[i]) {
        case '/':
        case '\\':
            path.x[length++] = '_';
            break;
        
        case '.':
            if(file->text[i+1] != '.') {
                last_dot = length;
                goto oh_normal;
            }
            i += 2;

            length += 5;
            if(length >= PATH_MAX) {
                cate_error("path too long to add \"back_\"");
            }
            strncat(path.x, "back_", 6);
            break;
        
        default:
        oh_normal:
            path.x[length++] = file->text[i];
            break;
        }
    }

    //replace extension
    length = last_dot + cate_target->object_ending.length+1;
    if(length >= PATH_MAX) {
        cate_error("path too long to add object extension");
    }

    path.x[last_dot] = 0;
    strncat(path.x,
        cate_target->object_ending.text, cate_target->object_ending.length+1);
    
    return st_save_s(&ctx.st, path.x);
}

static void save_separated(string_view* s, SavedStringIndexes* a) {
    if(s->text[0] == 0) return;
    char* tmp = s->text;
    STIndex idx = 0;
    for (size_t i = 0; i < s->length; ++i) {
        if(isspace(s->text[i])) {
            //skip whitespace
            while(isspace(s->text[i])) {
                s->text[i] = 0;
                ++i;
            }
            idx = st_save_s(&ctx.st, tmp);
            da_append((*a), idx);
            tmp = &s->text[i];
        }
    }

    idx = st_save_s(&ctx.st, tmp);
    da_append((*a), idx);
}

static void prepare_obj_files(CateClass* c, SavedStringIndexes* a) {
    a->size = 0;
    for (size_t i = 0; i < c->files.size; ++i) {
        string_view f = sv_from_cstr(
            st_get_str(&ctx.st, c->files.data[i])
        );

        STIndex obj = objectify_file(&c->build_dir, &f);
        da_append((*a), obj);
    }
}

static void prepare(CateClass* c, Prepared* p) {
    //TODO: make all of these pass-by-pointer
    p->out_name = prepare_out_name(c);
    p->standard = prepare_std(c);
    create_directories(c);
    prepare_obj_files(c, &p->object_files);

    if(c->kind == CLASS_LIBRARY) {
        char flags[17] = "-g -shared -fPIC";
        string_view lib_flags = {.length = 17, .text = flags};
        save_separated(&lib_flags, &p->flags);
    }
    save_separated(&c->flags, &p->flags);

    todo("prepare");
}

struct FileBuilder {
    struct CateSysProcess proc;
    CStringArray command;
};

static void create_build_process(struct FileBuilder* b,
            const char* f, const char* o, const CStringArray* t) {
    //If the command array exists, we shouldn't free it until the end.
    const char* null = 0;

    if(!b->command.data) {
        b->command.capacity = t->capacity;
        b->command.size = t->size;
        b->command.data = malloc(b->command.capacity);
        memcpy(b->command.data, t->data, t->size*sizeof(t->data[0]));
        da_append(b->command, null);
    } else {
        b->command.size -= 3;
    }

    da_pop(b->command);
    da_append(b->command, f);
    static const char* dash_o = "-o";
    da_append(b->command, dash_o);
    da_append(b->command, o);
    da_append(b->command, null);
    
    if(cmd_args.flags & CMD_DRY_RUN) return;
    b->proc = cate_sys_process_create(b->command.data);
}

void class_build(CateClass* c) {
    Prepared p = {0};
    prepare(c, &p);
    c->command_template.size = 0;
    
    //prepare command
    da_append(c->command_template, c->compiler);

    size_t chunk_size = (c->files.size < cmd_args.thread_count)
                        ? c->files.size
                        : cmd_args.thread_count;

    struct FileBuilder builders[chunk_size];
    memset(&builders, 0, sizeof(struct FileBuilder)*chunk_size);

    const STIndex* const files = c->files.data;
    size_t temp = 0;
    for (size_t i = 0; i < c->files.size; ++i) {
        const char* file =
            st_get_str(&ctx.st, files[i]);
        const char* obj =
            st_get_str(&ctx.st, p.object_files.data[i]);

        if(!cate_is_file_newer(file, obj)) continue;

        if(cmd_args.flags & CMD_DRY_RUN) {
            //since it's a dry run and reused, no need to free it
            static struct FileBuilder b = {0};
            create_build_process(&b,
                file, obj, &c->command_template);

            //-1 for the last null
            for (size_t i = 0; i < b.command.size-1; i++)
                printf("%s ", b.command.data[i]);
            printf("\n");
            continue;
        }
    }
    

    for (size_t i = 0; i < chunk_size; ++i) {
        free(builders[i].command.data);
    }
    free(p.object_files.data);
    free(p.flags.data);
    
    todo("building a class");
}

void class_clean(CateClass* c) {
    if(!c->objects.size) prepare_objects(c);

    for (size_t i = 0; i < c->objects.size; ++i) {
        char* path = st_get_str(&ctx.st, c->objects.data[i]);
        if(!cate_sys_remove(path))
            cate_error("failed to clean!");
    }
}

void class_install(CateClass* c) {
    todo("installing a class");
}
