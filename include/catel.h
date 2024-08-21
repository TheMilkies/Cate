#ifndef CATEL_H
#define CATEL_H
#include <vendor/string_view.h>
#include "system_functions.h"
#include "path_builder.h"
#include "target.h"

#define CATEL_PATH_MAX 256
#define CATEL_PATH_SIZE CATEL_PATH_MAX+6 //6 for the .cate
struct CatelValues {
    char dir[CATEL_PATH_SIZE], file[CATEL_PATH_SIZE];
    uint8_t has_file;
};

char* catel_build_path(struct CatePathBuilder* p,
                    char* file);
void catel_init();
extern struct CatelValues catel;

#endif // CATEL_H