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

static void prepare(CateClass* c, Prepared* p) {
    todo("prepare");
}

void class_build(CateClass* c) {
    todo("building a class");
    Prepared p = {0};
    prepare(c, &p);
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
