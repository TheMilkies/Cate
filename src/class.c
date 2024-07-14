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
        strncpy(f.x, ext->text, ext->length);
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
        strncpy(f.x, c->name.text, c->name.length);
        strncpy(f.x, ext->text, ext->length);
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
    strncpy(flag.x, c->standard.text, c->standard.length+1);
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

static void prepare(CateClass* c, Prepared* p) {
    p->out_name = prepare_out_name(c);
    p->standard = prepare_std(c);
    create_directories(c);
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
