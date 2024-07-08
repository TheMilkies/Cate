#include "string_table.h"

static size_t round_to_pow2(size_t n) {
    --n;
    //for portability
    for (size_t i = 1; i < sizeof(n)*8; i *= 2)
        n |= n >> i;

    ++n;
    return n;
}

void st_free(StringTable* st) {
    free(st->buf);
    memset(st, 0, sizeof(*st));
}

void st_reset(StringTable* st) {
    memset(st->buf, 0, st->length);
    st->length = 0;
}

STIndex st_save_s(StringTable* st, char* s) {
    string_view v = {.text = s, .length = strlen(s)};
    return st_save_sv(st, &v);
}

STIndex st_save_sv(StringTable* st, string_view* sv) {
    const size_t added_size = st->length + sv->length+1;
    const size_t needed_size = round_to_pow2(added_size);

    if(!st->buf) {
        st->buf = calloc(needed_size, sizeof(char));
        if(!st->buf) {
            fprintf(stderr, "\ncate can't allocate enough memory.\n");
            exit(-2);
        }
        st->capacity = needed_size;
    }

    if(needed_size > st->capacity) {
        st->buf = realloc(st->buf, needed_size);
        st->capacity = needed_size;
    }

    char* saved = st->buf+st->length;
    const size_t to_append = st->length;

    memcpy(saved, sv->text, sv->length);
    st->length += sv->length+1;
    //null terminate to be sure
    saved[sv->length+1] = 0;
    return (size_t)saved;
}