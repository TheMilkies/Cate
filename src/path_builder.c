#include "path_builder.h"
#include "error.h"
#include "class.h"
#include <assert.h>

static void pb_from_slen(struct CatePathBuilder* pb,
                            char* text, size_t length) {
    memcpy(pb->path.x, text, length);
    pb->length = length;
}

void pb_from_cstr(struct CatePathBuilder* pb, char* text) {
    pb_from_slen(pb, text, strlen(text));
}

void pb_from_sv(struct CatePathBuilder* pb, string_view* text) {
    pb_from_slen(pb, text->text, text->length);
}

void pb_reset(struct CatePathBuilder* pb) {
    memset(pb->path.x, 0, pb->length);
    pb->length = 0;
}

static void append(struct CatePathBuilder* pb, char* s, size_t l) {
    if(pb->length+l >= PATH_MAX)
        cate_error("path \"%s"sv_fmt"\" is too long?",
            pb->path.x, s, l);

    memcpy(&pb->path.x[pb->length], s, l);
    pb->length += l;
}

void pb_append(struct CatePathBuilder* pb, char* text) {
    append(pb, text, strlen(text));
}

void pb_append_sv(struct CatePathBuilder* pb, string_view* text) {
    append(pb, text->text, text->length);
}

void pb_append_cate_extension(struct CatePathBuilder* pb) {
    append(pb, ".cate", 6);
}

void pb_append_dir_sep(struct CatePathBuilder* pb) {
    append(pb, path_sep_str, path_sep_str_len-1);
}

void pb_revert_to(struct CatePathBuilder* pb, size_t old_loc) {
    assert(old_loc > pb->length && "reverting forwards is impossible");
    memset(&pb->path.x[old_loc], 0, PATH_MAX-old_loc);
    pb->length = old_loc;
}

STIndex pb_save(struct CatePathBuilder* pb) {
    st_save_slen(&ctx.st, pb->path.x, pb->length);
}