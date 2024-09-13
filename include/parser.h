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
    NODE_FOCUS_ON_OBJECT,
    NODE_GET_IMPLIED_PROPERTY,
    NODE_GET_PROPERTY,
    NODE_ASSIGN,
    NODE_FUNC_CALL,

    NODE_IF,
    NODE_BLOCK,
    NODE_NEGATE,
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
    AST expr_stack, oper_stack, inter_stack;
    size_t i;
    uint8_t object_selected;
} Parser;

AST cate_parse(Parser* p);
void parser_free(Parser* p);

#endif // CATE_PARSER_H