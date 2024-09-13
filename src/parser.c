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
static void warn(Parser* p, const char* text) {
    cate_warn_line(cur_tok.line, text);
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
static void expect_bool(Parser* p);
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

static void parse_tld(Parser* p);
AST cate_parse(Parser* p) {
    pre_alloc(&p->ast);
    p->i = 0;
    p->expr_stack.size = 0;
    p->object_selected = 0;
    while (in_range()) {
        parse_tld(p);
    }
    
    return p->ast;
}

static void parse_block(Parser* p);
static void parse_tld(Parser* p) {
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
        end_node(p, n);
        p->object_selected = 1;

        // if(!match(TOK_DOT))
        //     error("expected a dot after the identifier", 0);
        goto dot;
    }   break;

    case TOK_DOT: dot: {
        // if(!match(TOK_DOT))
        //     error("expected an identifier after the dot", 0);
        parse_expr(p);
    }   break;

    // case TOK_IF: {
    //     NodeID n = add_node(p, NODE_IF);
    //     next();
    //     parse_condition(p);

    //     if(match(TOK_ELSE)) {
    //         next();
    //         parse_block(p);
    //     }
    //     end_node(p, n);
    // }   break;

    default:
        error("did not expect %s", tok_as_text(cur_tok.kind));
        break;
    }
}

// static NodeID parse_condition(Parser* p) {
//     NodeID n = 0;
//     switch (cur_tok.kind) {
//     case TOK_LPAREN:
//         next();
//         n = parse_condition(p);
//         expect(TOK_RPAREN);
//         return n;
//         break;

//     case TOK_EXCLAMATION_MARK:
//         next();
//         n = add_node(p, NODE_NEGATE);
//         parse_condition(p);
//         break;
    
//     case TOK_IDENTIFIER:
//         parse_expr(p);
//         break;
    
//     default:
//         error("invalid condition", 0);
//         break;
//     }
//     end_node(p, n);
// }

static void parse_block(Parser* p) {
    NodeID n = add_node(p, NODE_BLOCK);
    expect(TOK_LCURLY);

    while(in_range() && !match(TOK_RCURLY)) {
        parse_tld(p);
    }

    if(!in_range())
        error("unclosed '{'", 0);
    expect(TOK_RCURLY);
    end_node(p, n);
}

static int get_precedence(Parser* p, TokenKind k) {
    switch (k) {
    case TOK_ASSIGN:
        return 0;
        break;
    
    default:
        error("invalid operator!", 0);
        break;
    }
}

static NodeKind tok_to_node(Parser* p) {
    switch (cur_tok.kind){
    case TOK_STRING_LITERAL:
        return NODE_STRING; break;
    case TOK_TRUE:
        return NODE_TRUE; break;
    case TOK_FALSE:
        return NODE_FALSE; break;
    
    default:
        error("%s is not a valid expression?",
            tok_as_text(cur_tok.kind));
        break;
    }
}

static ASTNode pop_node(AST* arr) {
    if(!arr->size) {
        cate_error("expected a value", 0);
    }

    ASTNode top = da_top(*arr);
    da_pop(*arr);
    return top;
}

static void convert_to_nodes(Parser* p);
static void stackify(Parser* p);
static void parse_expr(Parser* p) {
    //shunting yard
    stackify(p);
    convert_to_nodes(p);

    if(p->expr_stack.size == 1 && p->oper_stack.size == 0) {
        ASTNode n = pop_node(&p->expr_stack);
        da_append(p->ast, n);
    }

    //for debug, for now
    if(p->oper_stack.size) {
        error("unhandled %i on oper stack",
            da_top(p->oper_stack).node_kind);
    }

    if(p->expr_stack.size) {
        error("unhandled %i on expr stack",
            da_top(p->expr_stack).node_kind);
    }
}

static void stackify(Parser* p) {
    TokenKind last = TOK_NONE;
    uint8_t ident_used = 0;
    while(in_range()) {
        switch (cur_tok.kind) {
        case TOK_IDENTIFIER: {
            last = cur_tok.kind;
            ASTNode n = {
                .node_kind = NODE_IDENT,
                .token_index = p->i
            };
            da_append(p->expr_stack, n);
            next();
            ident_used = 1;
            if(!match(TOK_DOT)) continue;

        case TOK_DOT:
            ASTNode getter = {
                .node_kind =
                    (ident_used)
                    ? NODE_GET_PROPERTY
                    : NODE_GET_IMPLIED_PROPERTY,
                .token_index = p->i
            };
            expect(TOK_DOT);
            if(!p->object_selected && !ident_used)
                error("no object selected. define a Project?", 0);
            da_append(p->oper_stack, getter);

            ASTNode child = {
                .node_kind = NODE_IDENT,
                .token_index = p->i
            };
            da_append(p->expr_stack, child);
            expect(TOK_IDENTIFIER);
            continue;
        }   break;
        
        case TOK_ASSIGN: {
            last = cur_tok.kind;
            ASTNode n = {
                .node_kind = NODE_ASSIGN,
                .token_index = p->i
            };
            da_append(p->oper_stack, n);
            next();
        }   break;

        case TOK_STRING_LITERAL:
        case TOK_TRUE:
        case TOK_FALSE: {
            last = cur_tok.kind;
            ASTNode n = {
                .node_kind = tok_to_node(p),
                .token_index = p->i
            };
            da_append(p->expr_stack, n);
            next();
        }   break;
        
        default:
            error("expected an expression but got %s",
                tok_as_text(cur_tok.kind));
            break;
        }
    }
}

static inline NodeKind peek_stack(AST* stack) {
    return da_top(*stack).node_kind;
}

static void convert_to_nodes(Parser* p) {
    /* TODO: Some nodes depend on "future" nodes */
    size_t depends_on_next = 0;

while(p->oper_stack.size) {
    ASTNode op = pop_node(&p->oper_stack);
    switch (op.node_kind) {
    case NODE_GET_IMPLIED_PROPERTY: {
        ASTNode res = pop_node(&p->expr_stack);
        da_append(p->ast, op);
    }   break;

    case NODE_ASSIGN: {
        ASTNode rhs = pop_node(&p->expr_stack);
        ASTNode lhs = pop_node(&p->expr_stack);
        NodeID res = add_node(p, op.node_kind);
        da_append(p->ast, lhs);
        da_append(p->ast, rhs);
        end_node(p, res);
    }   break;

    case NODE_GET_PROPERTY: {
        ASTNode set_to = pop_node(&p->expr_stack);
        ASTNode prop = pop_node(&p->expr_stack);
        ASTNode obj = pop_node(&p->expr_stack);
        NodeID res = add_node(p, op.node_kind);
        da_append(p->ast, obj);
        da_append(p->ast, prop);
        da_append(p->ast, set_to);
        end_node(p, res);
    }   break;
    
    default:
        error("unhandled node: %i", op.node_kind);
        break;
    }
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

static void expect_bool(Parser* p) {
    if(cur_tok.kind != TOK_TRUE && cur_tok.kind != TOK_FALSE) {
        error("expected a boolean value ("
        traffic_light("true", " | ", "false")")", 0);
    }
    NodeKind k =
        cur_tok.kind == TOK_TRUE
        ? NODE_TRUE
        : NODE_FALSE;
    NodeID n = add_node(p, k);
    next();
    end_node(p, n);
}

static void optional_rparen(Parser* p) {
    if(match(TOK_RPAREN))
        next()
    else
        warn(p, "expected a ')' but i guess i don't need it");
}

static void in_parens(Parser* p, ParserRule rule) {
    expect(TOK_LPAREN);
    rule(p);
    optional_rparen(p);
}

void parser_free(Parser* p) {
    free(p->ast.data);
    free(p->expr_stack.data);
    free(p->oper_stack.data);
    free(p->inter_stack.data);
}