#include "class.h"
#include "system_functions.h"
#include "path_builder.h"
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

typedef struct {
    SavedStringIndexes object_files;
    SavedStringIndexes to_build_indexes;
    SavedStringIndexes flags, final_flags;
    STIndex standard;
    STIndex out_name;
} Prepared;

static void change_extension(struct CatePathBuilder* f, ClassKind k) {
    string_view* ext = (k == LIBRARY_STATIC)
            ? &cate_target->static_ending
            : &cate_target->dynamic_ending;
    string_view sv = {
        .length = f->length,
        .text = f->path.x
    };
    size_t last_dot = sv_find_last(&sv, '.');
    if(last_dot == SV_NOT_FOUND)
        last_dot = f->length-1;

    pb_revert_to(f, last_dot);
    pb_append_sv(f, ext);
}

static STIndex prepare_out_name(CateClass* c) {
    struct CatePathBuilder f = {0};
    if(c->out_name.length) {
        cate_sys_convert_path(c->out_name.text);
        pb_from_sv(&f, &c->out_name);
        if(c->bools & CLASS_IBOOL_TYPE_CHANGED)
            change_extension(&f, c->as.lib.kind);

        return pb_save(&f);
    }
    
    if(c->name.length + 1 >= PATH_MAX) {
        cate_error("object \""sv_fmt"\"'s name is too long for this platform!",
            sv_p(c->out_name));
    }

    string_view* ext = 0;
    switch (c->kind) {
    case CLASS_PROJECT: {
        ext = &cate_target->executable_ending;
        if(c->name.length + ext->length + 1 >= PATH_MAX) {
            cate_error("object \""sv_fmt"\"'s out name is too long!",
                sv_p(c->out_name));
        }
        pb_from_sv(&f, &c->name);
        pb_from_sv(&f, ext);
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

        static string_view out_slash = sv_from_const(
            "out" path_sep_str "lib"
        ); 
        pb_from_sv(&f, &out_slash);
        pb_append_sv(&f, &c->name);
        pb_append_sv(&f, ext);
    }   break;
    
    default:
        cate_error("not project or library, wtf");
        break;
    }
    return pb_save(&f);
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
    struct CatePathBuilder path = {0};
    if(dir->length + file->length >= PATH_MAX) {
        cate_error("object file length will be too long for \""sv_fmt"\"",
            svptr_p(file));
    }

    //add directory
    pb_from_sv(&path, dir);
    if(dir->text[dir->length-1] != path_sep)
        pb_append_dir_sep(&path);

    size_t begin = path.length;
    pb_append_sv(&path, file);
    size_t last_dot = 0, length = begin;
    for (size_t i = 0; i < file->length; ++i) {
        switch (file->text[i]) {
        case '/':
        case '\\':
            path.path.x[length++] = '_';
            break;
        
        case '.':
            if(file->text[i+1] != '.') {
                last_dot = length;
                goto oh_normal;
            }
            i += 2;

            pb_append_slen(&path, "back_", 5);
            break;
        
        default:
        oh_normal:
            path.path.x[length++] = file->text[i];
            break;
        }
    }

    if(last_dot)
        path.length = last_dot;
    pb_append_sv(&path, &cate_target->object_ending);
    return pb_save(&path);
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
        cate_sys_convert_path(f.text);

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

static void check_if_needs_rebuild(CateClass* c, Prepared* p,
                                string_view* out_name) {
    p->to_build_indexes.size = 0;
    for (size_t i = 0; i < c->files.size; ++i) {
        const char* file = st_get_str(&ctx.st, c->files.data[i]);
        const char* obj = st_get_str(&ctx.st, p->object_files.data[i]);
        if(cmd_args.flags & CMD_FORCE_REBUILD
        || cate_is_file_newer(file, obj)) {
            da_append(p->to_build_indexes, i);
        }
    }

    if(p->to_build_indexes.size
    || !cate_sys_file_exists(out_name->text))
        c->bools |= CLASS_IBOOL_RELINK;
}

//this is a dumb hack
static inline void carr_append(CStringArray* a, char* s) {
    da_append((*a), s);
}

static inline void ssi_append(SavedStringIndexes* a, char* s, size_t l) {
    STIndex i = st_save_slen(&ctx.st, s, l);
    da_append((*a), i);
}

static void prepare(CateClass* c, Prepared* p) {
    if(!ctx.builders) {
        ctx.builders = calloc(cmd_args.thread_count,
            sizeof(struct FileBuilder));
    }

    cate_sys_convert_path(c->build_dir.text);
    p->out_name = prepare_out_name(c);
    string_view out_name = sv_from_cstr(st_get_str(&ctx.st, p->out_name));
    prepare_obj_files(c, &p->object_files);
    check_if_needs_rebuild(c, p, &out_name);
    if(!(c->bools & CLASS_IBOOL_RELINK))
        return;

    p->standard = prepare_std(c);
    create_directories(c);

    if(c->kind == CLASS_LIBRARY) {
        ssi_append(&p->flags, "-g", 3);
        ssi_append(&p->flags, "-shared", 8);
        ssi_append(&p->flags, "-fPIC", 6);
    }
    if(c->flags.length)
        save_separated(&c->flags, &p->flags);
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
    } else {
        b->command.size -= 4;
    }

    static char* dash_o = "-o";
    //this appends "-o $o $f"
    da_append(b->command, dash_o);
    da_append(b->command, o);
    da_append(b->command, f);
    da_append(b->command, null);

    if(cmd_args.flags & CMD_DRY_RUN) return;
    b->proc = cate_sys_process_create(b->command.data);
}

static void dry_run_print(CStringArray* cmd) {
    for (size_t i = 0; i < cmd->size-1; ++i)
        printf("%s ", cmd->data[i]);
    printf("\n");
}

static void build(CateClass* c, Prepared* p) {
    size_t chunk_size = (c->files.size < cmd_args.thread_count)
                        ? c->files.size
                        : cmd_args.thread_count;

    size_t built_count = 0;
    while (built_count < p->to_build_indexes.size) {
        for (size_t thr = 0; thr < chunk_size; ++thr) {
            struct FileBuilder* builder = &ctx.builders[thr];
            if(cate_sys_has_process_exited(&builder->proc)) {
                if(builder->proc.exit_code)
                    cate_error("error in build command!");
                builder->proc.id = 0;

                size_t file_index = p->to_build_indexes.data[built_count++];
                if(built_count > p->to_build_indexes.size)
                    //we're done, break and wait for the others.
                    break;

                const char* file = st_get_str(&ctx.st,
                        c->files.data[file_index]);
                const char* obj = st_get_str(&ctx.st,
                        p->object_files.data[file_index]);

                create_build_process(builder, file, obj, &c->command_template);
                if(cmd_args.flags & CMD_DRY_RUN)
                    dry_run_print(&builder->command);
            }
        }
    }

    //wait for the ones that didn't end
    if(cmd_args.flags & CMD_DRY_RUN) return;
    while (1) {
        uint8_t running = 0;
        for (size_t thr = 0; thr < chunk_size; ++thr) {
            struct FileBuilder* builder = &ctx.builders[thr];
            if(!builder->proc.id) continue;
            
            if(cate_sys_has_process_exited(&builder->proc)) {
                if(builder->proc.exit_code)
                    cate_error("error in build command!");
                builder->proc.id = 0;
            }
            running = 1;
        }

        if(!running) return;
    }
}

static void prepare_command_template(CateClass* c, Prepared* p) {
    static const char *dash_c = "-c";
    c->command_template.size = 0;
    cate_sys_convert_path(c->compiler.text);
    da_append(c->command_template, c->compiler);
    append_ssi_items(&c->command_template, &p->flags);
    append_ssi_items(&c->command_template, &c->includes);

    //we remove the -c later by just popping
    da_append(c->command_template, dash_c);
}

static void run_builder_and_wait(struct FileBuilder* b) {
    if(cmd_args.flags & CMD_DRY_RUN) {
        dry_run_print(&b->command);
    } else {
        b->proc = cate_sys_process_create(b->command.data);
        int err = cate_sys_process_wait(&b->proc);
        if(err)
            cate_error("build command exited with code %i", err);
    }
}

static void link_static_library(CateClass* c, Prepared* p) {
    struct FileBuilder final = {0};
    static char* null = 0;
    carr_append(&final.command, "ar");
    carr_append(&final.command, "rcs");

    char* out_name = st_get_str(&ctx.st, p->out_name);
    da_append(final.command, out_name);

    append_ssi_items(&final.command, &p->object_files);
    da_append(final.command, null);

    run_builder_and_wait(&final);
}

static void link(CateClass* c, Prepared* p) {
    static char* dash_o = "-o", *null = 0;
    if(c->kind == CLASS_LIBRARY && c->as.lib.kind == LIBRARY_STATIC)
        return link_static_library(c, p);

    struct FileBuilder final = {0};
    copy_cstring_array(&final.command, &c->command_template);
    if(c->final_flags.length) {
        save_separated(&c->final_flags, &p->final_flags);
        append_ssi_items(&final.command, &p->final_flags);
    }
    append_ssi_items(&final.command, &p->object_files);

    da_append(final.command, dash_o);
    char* out_name = st_get_str(&ctx.st, p->out_name);
    da_append(final.command, out_name);
    da_append(final.command, null);

    //final step
    if(c->bools & CLASS_IBOOL_RELINK)
        run_builder_and_wait(&final);
    
    if(c->bools & CLASS_BOOL_SMOL || cmd_args.flags & CMD_FORCE_SMOLIZE) {
        cate_sys_smolize(c->out_name.text);
    }
    free(final.command.data);
}

void class_build(CateClass* c) {
    if(c->bools & CLASS_BOOL_BUILT
    && !(cmd_args.flags & CMD_FORCE_REBUILD))
        return;

    Prepared p = {0};
    prepare(c, &p);
    if(c->bools & CLASS_IBOOL_RELINK) {
        prepare_command_template(c, &p);
        build(c, &p);
        //pop the -c
        da_pop(c->command_template);
        link(c, &p);
    }
    
    //finished building, let's mark it as built
    c->bools |= CLASS_BOOL_BUILT;
    //free
    free(p.object_files.data);
    free(p.flags.data);
    free(p.final_flags.data);
    free(p.to_build_indexes.data);
    builders_reset();
}

void class_clean(CateClass* c) {
    if(!c->objects.size) prepare_obj_files(c, &c->objects);

    for (size_t i = 0; i < c->objects.size; ++i) {
        char* path = st_get_str(&ctx.st, c->objects.data[i]);
        if(!cate_sys_remove(path))
            cate_error("failed to clean!");
    }
}

void class_change_type(CateClass* c, LibraryKind type) {
    if(c->kind != CLASS_LIBRARY) return;
    c->as.lib.kind = type;
    c->bools |= CLASS_IBOOL_TYPE_CHANGED;
    c->bools &= ~(CLASS_BOOL_BUILT);
}

void class_install(CateClass* c) {
    todo("installing a class");
}