#ifndef CATE_RECURSIVE_H
#define CATE_RECURSIVE_H
#include "system_functions.h"
#include "string_table.h"
#include <stdint.h>

enum {
    RECURSIVE_GET_NONE = 0,
    RECURSIVE_GET_FILES = 1 << 0,
    RECURSIVE_GET_FILES_WITH_EXT = 1 << 1,
    RECURSIVE_GET_DIRS = 1 << 2,
    RECURSIVE_GET_EVERYTHING = 
        RECURSIVE_GET_FILES | RECURSIVE_GET_DIRS,
};

typedef struct {
    string_view extension;
    SavedStringIndexes* arr;

    uint8_t to_get;
    uint8_t subrecursion;
} RecursiveData;

int cate_recursive(RecursiveData* data, string_view* path);

#endif // CATE_RECURSIVE_H