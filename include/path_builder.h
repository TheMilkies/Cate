#ifndef CATE_PATH_BUILDER_H
#define CATE_PATH_BUILDER_H
#include "string_table.h"
#include "system_functions.h"
#include "target.h"

struct CatePathBuilder {
    struct CateFullPath path;
    size_t length;
};

void pb_from_cstr(struct CatePathBuilder* pb, char* text);
void pb_from_sv(struct CatePathBuilder* pb, string_view* text);
void pb_reset(struct CatePathBuilder* pb);
void pb_append_slen(struct CatePathBuilder* pb, char* s, size_t l);
void pb_revert_to(struct CatePathBuilder* pb, size_t old_loc);
void pb_append(struct CatePathBuilder* pb, char* text);
void pb_append_sv(struct CatePathBuilder* pb, string_view* text);
void pb_append_cate_extension(struct CatePathBuilder* pb);
void pb_append_dir_sep(struct CatePathBuilder* pb);
STIndex pb_save(struct CatePathBuilder* pb);

#endif // CATE_PATH_BUILDER_H