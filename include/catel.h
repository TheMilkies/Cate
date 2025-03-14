#ifndef CATE_CATEL_H
#define CATE_CATEL_H
#include "cate.h"
#include "tokenizer.h"

typedef struct {
    CateSysPath dir, def;
} CatelFile;

void catel_run();
extern CatelFile catel;

#endif // CATE_CATEL_H