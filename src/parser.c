#include "parser.h"

#define tok_get(idx) p->kinds.data[idx]
#define tok_value(idx) p->value.data[idx]
#define cur_tok tok_get(p->i)
#define cur_value tok_value(p->i)

#define next() ++p->i;
#define in_range() (p->i < p->kinds.size)
#define match(k) (cur_tok.kind == k)

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
static void expect_library_kind(Parser* p);
typedef void(*ParserRule)(Parser*);
static void in_parens(Parser* p, ParserRule rule);

//instead of having a nesting tree, we have a flat array
static TokenID add_node(Parser* p, NodeKind kind) {
    ASTNode n = {
        .token_index = p->i,
        .node_kind = kind
    };
    da_append(p->ast, n);
    return p->ast.size-1;
}

static void change_node_kind(Parser* p, NodeID id, NodeKind k) {
    p->ast.data[id].node_kind = k;
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
    p->object_selected = 0;
    while (in_range()) {
        switch (cur_tok.kind) {
        case TOK_PROJECT: {
            NodeID n = add_node(p, NODE_DEF_PROJECT);
            next();
            //name
            expect_ident(p);
            end_node(p, n);
            p->object_selected = 1;
        }   break;

        case TOK_LIBRARY: {
            NodeID n = add_node(p, NODE_DEF_LIBRARY);
            next();
            //name
            expect_ident(p);
            //(type)
            in_parens(p, expect_library_kind);
            end_node(p, n);
            p->object_selected = 1;
        }   break;

        case TOK_IDENTIFIER: {
            NodeID n = add_node(p, NODE_FOCUS_ON_OBJECT);
            next();
            end_node(p, n);
            p->object_selected = 1;
            
            if(!match(TOK_DOT))
                error("expected a dot after the identifier", 0);
            goto dot;
        }   break;

        case TOK_DOT: dot: {
            parse_expr(p);
        }   break;

        default:
            error("unexpected %s", tok_as_text(cur_tok.kind));
            break;
        }
    }
    
    return p->ast;
}

static void parse_expr(Parser* p) {
    NodeID n = 0;
    switch (cur_tok.kind) {
    case TOK_IDENTIFIER: {
        n = add_node(p, NODE_GET_PROPERTY);
        next();
        goto dot;
    }   break;

    case TOK_DOT: {
        if(!n)
            n = add_node(p, NODE_ASSIGN);

    dot:
        expect(TOK_DOT);
        if(!p->object_selected)
            error("no object selected. define a Project?", 0);
        expect_ident(p);

        if(match(TOK_ASSIGN)) {
            change_node_kind(p, n, NODE_ASSIGN);
            next();
            parse_expr(p);
        }
        end_node(p, n);
    }   break;

    case TOK_STRING_LITERAL:
        expect_string(p);
        break;

    default:
        break;
    }
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

static void expect_library_kind(Parser* p) {
    if(cur_tok.kind != TOK_STATIC && cur_tok.kind != TOK_DYNAMIC) {
        error("expected a "hl_var("LibraryType")" value ("
        choose_light("static", " | ", "dynamic")")",0);
    }
    NodeKind k =
        cur_tok.kind == TOK_DYNAMIC
        ? NODE_DYNAMIC
        : NODE_STATIC;
    NodeID n = add_node(p, k);
    next();
    end_node(p, n);
}

static void in_parens(Parser* p, ParserRule rule) {
    expect(TOK_LPAREN);
    rule(p);
    expect(TOK_RPAREN);
}