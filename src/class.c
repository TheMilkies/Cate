#include "class.h"
#include "system_functions.h"
#include "common.h"
#include "error.h"

static void classes_free() {
    for (size_t i = 0; i < ctx.classes.size; i++) {
        CateClass* c = &ctx.classes.data[i];
        free(c->includes.data);
        free(c->defines.data);
        free(c->files.data);
        free(c->objects.data);
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
    p->out_name = prepare_out_name(c);
    p->standard = prepare_std(c);
    create_directories(c);
    prepare_obj_files(c, &p->object_files);
    todo("prepare");
}

void class_build(CateClass* c) {
    Prepared p = {0};
    prepare(c, &p);
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
