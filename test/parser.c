#include "parser.h"
#include "testlib.h"

#define parse(text)\
    string_view line = sv_from_const(text);\
    Parser p = {0};\
    cate_tokenize(&line, &p.kinds, &p.values);\
    AST ast = cate_parse(&p);

#define assert_k(a, b) _assert((a.node_kind == b), #a " != " #b " (is %i)",\
    a.node_kind)

#define assert_v(a, b, blen) _assert(sv_equalc(&p.values.data[a.token_index],\
                                b, blen),\
    #a " != " #b " (is \""sv_fmt"\")",\
    sv_p(p.values.data[a.token_index]));

#define cleanup() free(toks.data); free(vals.data); parser_free(&p);

test_def(project_init) {
    parse("Project hello");
    assert_k(ast.data[0], NODE_DEF_PROJECT);
    assert_k(ast.data[1], NODE_IDENT);
    assert_v(ast.data[1], "hello", 5);
}

test_def(library_init) {
    parse("Library hello(static)\n");
    assert_k(ast.data[0], NODE_DEF_LIBRARY);
    assert_k(ast.data[1], NODE_IDENT);
    assert_v(ast.data[1], "hello", 5);
    assert_k(ast.data[2], NODE_STATIC);
}

test_def(property_get) {
    parse("Project x\n"
          ".out = \"hello\"");
    
    assert_k(ast.data[2], NODE_ASSIGN);
    assert_k(ast.data[3], NODE_GET_IMPLIED_PROPERTY);
    assert_k(ast.data[4], NODE_IDENT);
    assert_v(ast.data[4], "out", 3);
    assert_k(ast.data[5], NODE_STRING);
    assert_v(ast.data[5], "hello", 5);
}

test_def(expr) {
    parse("x = \"hello\"");
    assert_k(ast.data[0], NODE_FOCUS_ON_OBJECT);
    assert_k(ast.data[1], NODE_ASSIGN);
    assert_eq(ast.data[1].nodes_right, 3);
    assert_k(ast.data[2], NODE_IDENT);
    assert_v(ast.data[2], "x", 1);
    assert_k(ast.data[3], NODE_STRING);
    assert_v(ast.data[3], "hello", 5);
}

test_def(property_get_another) {
    parse("Project x1\n"
          ".out = x2.out");
    assert_k(ast.data[2], NODE_ASSIGN);
    assert_k(ast.data[3], NODE_IDENT);
    assert_v(ast.data[3], "out", 3);
    assert_k(ast.data[4], NODE_GET_PROPERTY);
    assert_v(ast.data[4], "x2", 2);
    assert_k(ast.data[5], NODE_IDENT);
    assert_v(ast.data[5], "out", 3);
}

test_main({
    // test_register(project_init);
    // test_register(library_init);
    // test_register(expr);
    test_register(property_get);
    // test_register(property_get_another);
})