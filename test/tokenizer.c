#include "tokenizer.h"
#include "testlib.h"

#define tokenize(text)\
    string_view line = sv_from_const(text);\
    TokensArray toks = {0};\
    TokenValuesArray vals = {0};\
    cate_tokenize(&line, &toks, &vals);

#define cleanup() free(toks.data); free(vals.data);

test_def(idents) {
    tokenize("hi hello123");
    assert_eq(toks.size, vals.size);
    assert_eq(toks.size, 2);
    assert_sv_eqc(vals.data[0], "hi", 2);
    assert_sv_eqc(vals.data[1], "hello123", 8);

    cleanup();
}

test_def(whitespace) {
    tokenize("  hello\n\ttest\t ");
    assert_eq(toks.size, 2);
    assert_sv_eqc(vals.data[0], "hello", 5);
    assert_sv_eqc(vals.data[1], "test", 4);

    cleanup();
}

test_main({
    test_register(idents);
    test_register(whitespace);
})