#ifndef CATEL_H
#define CATEL_H
#include <vendor/string_view.h>
#include "system_functions.h"
#include "path_builder.h"
#include "target.h"

#define CATEL_PATH_MAX 256
#define CATEL_PATH_SIZE CATEL_PATH_MAX+6 //5 for the .cate
typedef struct {
    char dir[CATEL_PATH_SIZE], file[CATEL_PATH_SIZE];
    struct CateFullPath file_path;
    uint8_t has_file;
} CatelValues;

void catel_build_path(struct CatePathBuilder* p, CatelValues* v,
                        string_view* file);
void catel_init(CatelValues* c);
extern CatelValues catel;

#endif // CATEL_H