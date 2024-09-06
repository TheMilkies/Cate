#include "parser.h"

#define tok_get(idx) p->kinds.data[idx]
#define tok_value(idx) p->value.data[idx]
#define cur_tok tok_get(p->i)
#define cur_value tok_value(p->i)

#define next() ++p->i;
#define in_range() (p->i < p->kinds.size)

static void error(Parser* p, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    cate_error_line_va(cur_tok.line, fmt, args);
}
#define error(fmt, ...) error(p, fmt, __VA_ARGS__)

static TokenID expect(Parser* p, TokenKind k) {
    TokenID backup = p->i;
    if(cur_tok.kind != k) {
        error("expected %s but got %s",
            tok_as_text(k), tok_as_text(cur_tok.kind));
    }
    next();
    return backup;
}
#define expect(k) expect(p, k)

static void parse_expr(Parser* p);
static void expect_string(Parser* p);
static void expect_ident(Parser* p);

//instead of having a nesting tree, we have a flat array
static TokenID add_node(Parser* p, uint8_t kind) {
    ASTNode n = {
        .token_index = p->i,
        .node_kind = kind
    };
    da_append(p->ast, n);
    return p->ast.size-1;
}

static void end_node(Parser* p, NodeID id) {
    ASTNode* n = &p->ast.data[id];
    n->nodes_right = p->ast.size-id;
}

static void pre_alloc(AST* ast) {
    if(ast->data) {
        ast->size = 0;
        return;
    }

    ast->capacity = sizeof(ASTNode)*128;
    ast->data = calloc(1, ast->capacity);
}

AST cate_parse(Parser* p) {
    pre_alloc(&p->ast);
    p->i = 0;
    while (in_range()) {
        switch (cur_tok.kind) {
        case TOK_PROJECT: {
            NodeID n = add_node(p, NODE_DEF_PROJECT);
            next();
            expect_ident(p);
            end_node(p, n);
        }   break;

        default:
            error("unexpected %s", tok_as_text(cur_tok.kind));
            break;
        }
    }
    
    return p->ast;
}

static void expect_ident(Parser* p) {
    NodeID n = add_node(p, NODE_IDENT);
    expect(TOK_IDENTIFIER);
    end_node(p, n);
}

static void expect_string(Parser* p) {
    NodeID n = add_node(p, NODE_STRING);
    expect(TOK_STRING_LITERAL);
    end_node(p, n);
}

static void parse_expr(Parser* p) {
    
}