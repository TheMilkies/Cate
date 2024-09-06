#include "tokenizer.h"
#include "testlib.h"

#define tokenize(text)\
    string_view line = sv_from_const(text);\
    TokensArray toks = {0};\
    TokenValuesArray vals = {0};\
    cate_tokenize(&line, &toks, &vals);

#define cleanup() free(toks.data); free(vals.data);
#define assert_k(a, b) _assert((a.kind == b), #a " != " #b " (is %s)",\
    tok_as_text(a.kind))

test_def(idents) {
    tokenize("hi hello123");
    assert_eq(toks.size, vals.size);
    assert_eq(toks.size, 2);

    assert_k(toks.data[0], TOK_IDENTIFIER);
    assert_k(toks.data[1], TOK_IDENTIFIER);

    assert_sv_eqc(vals.data[0], "hi", 2);
    assert_sv_eqc(vals.data[1], "hello123", 8);

    cleanup();
}

test_def(keywords) {
    tokenize("Project ifs");
    assert_eq(toks.size, 2);

    assert_k(toks.data[0], TOK_PROJECT);
    assert_k(toks.data[1], TOK_IDENTIFIER);

    assert_sv_eqc(vals.data[1], "ifs", 3);

    cleanup();
}

test_def(whitespace) {
    tokenize("  hello\n\ttest\t ");
    assert_eq(toks.size, 2);
    assert_k(toks.data[0], TOK_IDENTIFIER);
    assert_k(toks.data[1], TOK_IDENTIFIER);
    assert_sv_eqc(vals.data[0], "hello", 5);
    assert_sv_eqc(vals.data[1], "test", 4);

    cleanup();
}

test_def(comments) {
    tokenize("this/*hm*/works\n//test\nhello");
    assert_eq(toks.size, 3);
    assert_k(toks.data[0], TOK_IDENTIFIER);
    assert_k(toks.data[1], TOK_IDENTIFIER);
    assert_k(toks.data[2], TOK_IDENTIFIER);
    assert_sv_eqc(vals.data[0], "this", 4);
    assert_sv_eqc(vals.data[1], "works", 5);
    assert_sv_eqc(vals.data[2], "hello", 5);

    cleanup();
}

test_def(operators) {
    tokenize("={}");
    assert_eq(toks.size, 3);
    assert_k(toks.data[0], TOK_ASSIGN);
    assert_k(toks.data[1], TOK_LCURLY);
    assert_k(toks.data[2], TOK_RCURLY);

    cleanup();
}

test_def(strings) {
    tokenize("{\"why,\", \"hello there\", \"old sport\"}");
    assert_eq(toks.size, 5);
    assert_k(toks.data[0], TOK_LCURLY);
    assert_k(toks.data[1], TOK_STRING_LITERAL);
    assert_k(toks.data[2], TOK_STRING_LITERAL);
    assert_k(toks.data[3], TOK_STRING_LITERAL);
    assert_k(toks.data[4], TOK_RCURLY);

    assert_sv_eqc(vals.data[1], "why,", 4);
    assert_sv_eqc(vals.data[2], "hello there", 11);
    assert_sv_eqc(vals.data[3], "old sport", 9);

    cleanup();
}

test_main({
    test_register(idents);
    test_register(keywords);
    test_register(whitespace);
    test_register(comments);
    test_register(operators);
    test_register(strings);
})