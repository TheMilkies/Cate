#ifndef CATE_PARSER_H
#define CATE_PARSER_H
#include "class.h"
#include "tokenizer.h"

struct Globals {
    string_view compiler, standard;
    string_view build_dir;
    ClassBools bools;
};

typedef struct {
    struct Globals globals;
    TokensArray tokens;
    CateClass* cur_class;
    Token* cur;
    size_t i;
} Parser;

void cate_open(const char* path);
void parse(Parser* p);

#endif // CATE_PARSER_H