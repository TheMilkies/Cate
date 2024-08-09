#include "class.h"
#include "system_functions.h"
#include "common.h"
#include "cmd_args.h"
#include "error.h"
#include <ctype.h>

//TODO: clean this whole file up!

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

struct FileBuilder {
    struct CateSysProcess proc;
    CStringArray command;
};

static void builders_reset() {
    if(!ctx.builders) return;
    for (size_t i = 0; i < cmd_args.thread_count; ++i) {
        ctx.builders[i].command.size = 0;
        memset(&ctx.builders[i].proc, 0, sizeof(ctx.builders[i].proc));
    }
}

static void builders_free() {
    if(!ctx.builders) return;
    for (size_t i = 0; i < cmd_args.thread_count; ++i)
        free(ctx.builders[i].command.data);
    free(ctx.builders);
    ctx.builders = 0;
}

void context_reset() {
    classes_free();
    builders_reset();
    ctx.loaded_files.size = 0;
    st_reset(&ctx.st);
}

void context_free() {
    classes_free();
    builders_free();
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
    SavedStringIndexes to_build_indexes;
    SavedStringIndexes flags, final_flags;
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
    if(!a || !s || s->text[0] == 0) return;
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

static void prepare_obj_files(CateClass* c,
                                SavedStringIndexes* result) {
    result->size = 0;
    for (size_t i = 0; i < c->files.size; ++i) {
        string_view f = sv_from_cstr(
            st_get_str(&ctx.st, c->files.data[i])
        );

        STIndex obj = objectify_file(&c->build_dir, &f);
        da_append((*result), obj);
    }
}

static void append_ssi_items(CStringArray* dest,
                            const SavedStringIndexes* src) {
    for (size_t i = 0; i < src->size; ++i) {
        char* s = st_get_str(&ctx.st, src->data[i]);
        da_append((*dest), s);
    }
}

static void check_if_needs_rebuild(CateClass* c, Prepared* p) {
    p->to_build_indexes.size = 0;
    for (size_t i = 0; i < c->files.size; ++i) {
        const char* file = st_get_str(&ctx.st, c->files.data[i]);
        const char* obj = st_get_str(&ctx.st, p->object_files.data[i]);
        if(cmd_args.flags & CMD_FORCE_REBUILD
        || cate_is_file_newer(file, obj)) {
            da_append(p->to_build_indexes, i);
        }
    }

    if(p->to_build_indexes.size)
        c->bools |= CLASS_BOOL_RELINK;
}

static void prepare(CateClass* c, Prepared* p) {
    if(!ctx.builders) {
        ctx.builders = calloc(cmd_args.thread_count,
            sizeof(struct FileBuilder));
    }

    p->out_name = prepare_out_name(c);
    p->standard = prepare_std(c);
    create_directories(c);
    prepare_obj_files(c, &p->object_files);
    check_if_needs_rebuild(c, p);

    if(c->kind == CLASS_LIBRARY) {
        char flags[17] = "-g -shared -fPIC";
        string_view lib_flags = {.length = 17, .text = flags};
        save_separated(&lib_flags, &p->flags);
    }
    save_separated(&c->flags, &p->flags);

    // todo("prepare");
}

static void copy_cstring_array(CStringArray* dest, const CStringArray* src) {
    if(dest->data && dest->capacity < src->capacity)
        dest->data = realloc(dest->data, src->capacity);

    dest->capacity = src->capacity;
    dest->size = src->size;
    if(!dest->data) 
        dest->data = malloc(src->capacity);

    memcpy(dest->data, src->data, src->size*sizeof(src->data[0]));
}

static void create_build_process(struct FileBuilder* b,
            const char* f, const char* o, const CStringArray* t) {
    //If the command array exists, we shouldn't free it until the end.
    const char* null = 0;

    if(!b->command.data || !b->command.size) {
        copy_cstring_array(&b->command, t);
        da_append(b->command, null);
    } else {
        b->command.size -= 3;
    }

    da_pop(b->command);
    da_append(b->command, f);
    static char* dash_o = "-o";
    da_append(b->command, dash_o);
    da_append(b->command, o);
    da_append(b->command, null);
    
    if(cmd_args.flags & CMD_DRY_RUN) return;
    b->proc = cate_sys_process_create(b->command.data);
}

static void dry_run_print(CStringArray* cmd) {
    for (size_t i = 0; i < cmd->size-1; ++i)
        printf("%s ", cmd->data[i]);
    printf("\n");
}

void class_build(CateClass* c) {
    if(c->bools & CLASS_BOOL_BUILT
        && !(cmd_args.flags & CMD_FORCE_REBUILD))
        return;
    static char* dash_o = "-o", *dash_c = "-c", *null = 0;
    Prepared p = {0};
    prepare(c, &p);
    
    //prepare command
    c->command_template.size = 0;
    da_append(c->command_template, c->compiler);
    append_ssi_items(&c->command_template, &p.flags);
    //TODO: fix the includes
    append_ssi_items(&c->command_template, &c->includes);

    //we remove the -c later by just popping
    da_append(c->command_template, dash_c);

    size_t chunk_size = (c->files.size < cmd_args.thread_count)
                        ? c->files.size
                        : cmd_args.thread_count;

    const STIndex* const files = c->files.data;
    // size_t temp = 0;

    size_t built_count = 0;
    while (built_count < p.to_build_indexes.size) {
        for (size_t thr = 0; thr < chunk_size; thr++) {
            struct FileBuilder* builder = &ctx.builders[thr];
            if(cate_sys_has_process_exited(&builder->proc)) {
                if(builder->proc.exit_code)
                    cate_error("error in build command!");
                builder->proc.id = 0;

                size_t file_index = p.to_build_indexes.data[built_count++];
                if(built_count > p.to_build_indexes.size)
                    continue;

                const char* file = st_get_str(&ctx.st,
                        c->files.data[file_index]);
                const char* obj = st_get_str(&ctx.st,
                        p.object_files.data[file_index]);
                
                if(cmd_args.flags & CMD_DRY_RUN) {
                    create_build_process(&ctx.builders[0],
                        file, obj, &c->command_template);

                    dry_run_print(&ctx.builders[0].command);
                    continue;
                }
                create_build_process(builder, file, obj, &c->command_template);
            }
        }
    }
    
    //wait for the ones that didn't end
    while (1) {
        uint8_t running = 0;
        for (size_t thr = 0; thr < chunk_size; thr++) {
            struct FileBuilder* builder = &ctx.builders[thr];
            if(builder->proc.id) {
                if(cate_sys_has_process_exited(&builder->proc)) {
                    if(builder->proc.exit_code)
                        cate_error("error in build command!");
                    builder->proc.id = 0;
                }
                running = 1;
            }
        }

        if(!running) break;
    }

    //build the final command
    //pop the -c
    da_pop(c->command_template);
    struct FileBuilder final = {0};
    copy_cstring_array(&final.command, &c->command_template);
    if(c->final_flags.length) {
        save_separated(&c->final_flags, &p.final_flags);
        append_ssi_items(&final.command, &p.final_flags);
    }
    append_ssi_items(&final.command, &p.object_files);

    da_append(final.command, dash_o);
    char* out_name = st_get_str(&ctx.st, p.out_name);
    da_append(final.command, out_name);
    da_append(final.command, null);

    //final step
    if(c->bools & CLASS_BOOL_RELINK)  {
        if(cmd_args.flags & CMD_DRY_RUN) {
            dry_run_print(&final.command);
        } else {
            final.proc = cate_sys_process_create(final.command.data);
            int err = cate_sys_process_wait(&final.proc);
            if(err)
                cate_error("build command exited with code %i", err);
        }
    }
    //finished building, let's mark it as built
    c->bools |= CLASS_BOOL_BUILT;

    //free
    free(final.command.data);
    free(p.object_files.data);
    free(p.flags.data);
    free(p.final_flags.data);
    free(p.to_build_indexes.data);
    builders_reset();

    // todo("building a class");
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