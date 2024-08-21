#ifndef CATE_STRING_TABLE_H
#define CATE_STRING_TABLE_H
#include <vendor/dynamic_array.h>
#include <vendor/string_view.h>

/*
    this string table (ST) is a really dumb hack. it exists to save strings.
    it returns indexes because saving strings is hell otherwise.
    one moment it's valid, then reallocation happens and boom!
    invalid memory!
    ugh...

    (Adapted from an older project)
*/

typedef size_t STIndex;
typedef da_type(STIndex) SavedStringIndexes;
typedef da_type(char*) CStringArray;

typedef struct StringTable {
    char* buf;
    size_t capacity, length;
} StringTable;

STIndex st_save_s(StringTable* st, char* s);
STIndex st_save_slen(StringTable* st, char* s, size_t l);
STIndex st_save_sv(StringTable* st, string_view* sv);
void st_init(StringTable* st, size_t initial_size);
void st_free(StringTable* st);
void st_reset(StringTable* st);

static inline char* st_get_str(StringTable* st, STIndex index) {
    return &st->buf[index];
}

#endif // CATE_STRING_TABLE_H