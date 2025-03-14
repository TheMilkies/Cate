#ifndef CATE_PARSER_H
#define CATE_PARSER_H
#include "tokenizer.h"
#include "cate.h"
#include "catel.h"

typedef struct {
    da_type(CateClass) classes;
    da_type(CateSysPath) opened_files;
} CateContext;

void cate_run(CateContext* ctx, string_view file_name);

#endif // CATE_PARSER_H