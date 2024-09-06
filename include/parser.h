#ifndef CATE_PARSER_H
#define CATE_PARSER_H
#include "tokenizer.h"

enum {
    NODE_NONE = 0,
    NODE_IDENT,
    NODE_STRING,

    NODE_DEF_PROJECT,
    NODE_DEF_LIBRARY,
    NODE_STATIC,
    NODE_DYNAMIC,

    NODE_TRUE,
    NODE_FALSE,
};
typedef uint8_t NodeKind;

typedef struct {
    uint64_t token_index : 48,
             nodes_right : 16;
    NodeKind node_kind;
} ASTNode;

typedef size_t NodeID;
typedef da_type(ASTNode) AST;

typedef struct {
    TokensArray kinds;
    TokenValuesArray values;
    AST ast;
    size_t i;
} Parser;

AST cate_parse(Parser* p);

#endif // CATE_PARSER_H