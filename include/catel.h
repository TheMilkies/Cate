#ifndef CATEL_H
#define CATEL_H
#include <vendor/string_view.h>

#define CATEL_PATH_MAX 256
#define CATEL_PATH_SIZE 256+5 //5 for the .cate
typedef struct {
    char dir[CATEL_PATH_SIZE], file[CATEL_PATH_SIZE];
} CatelValues;

void catel_init(CatelValues* c);

#endif // CATEL_H