#include "parser.h"
#include "testlib.h"

#define parse(text)\
    string_view line = sv_from_const(text);\
    Parser p = {0};\
    cate_tokenize(&line, &p.kinds, &p.values);\
    AST ast = cate_parse(&p);

#define assert_k(a, b) _assert((a.node_kind == b), #a " != " #b " (is %s)",\
    a.node_kind)

#define assert_v(a, b, blen) _assert(sv_equalc(&p.values.data[a.token_index],\
                                b, blen),\
    #a " != " #b " (is \""sv_fmt"\")",\
    sv_p(p.values.data[a.token_index]));

#define cleanup() free(toks.data); free(vals.data);

test_def(project_init) {
    parse("Project hello");
    assert_k(ast.data[0], NODE_DEF_PROJECT);
    assert_k(ast.data[1], NODE_IDENT);
    assert_v(ast.data[1], "hello", 5);
}

test_main({
    test_register(project_init);
})