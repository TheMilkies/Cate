#include "class.h"

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