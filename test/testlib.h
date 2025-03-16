#ifndef CATE_TESTLIB_H
#define CATE_TESTLIB_H
#define SIMPLE_SV_IMPL
#define SIMPLE_DA_IMPL
#include <vendor/string_view.h>
#include <vendor/dynamic_array.h>
#define BOLD_RED "\033[1;31m"
#define COLOR_RESET "\033[0m"

struct _TestContext {
    const char* name;
    size_t total, error;
};
typedef void(*_TestFunction)(struct _TestContext*);
struct _Test {
    _TestFunction f;
    struct _TestContext c;
};

da_type(struct _Test) _tests = {0};

#define _test_name(name) _TEST__ ## name ## _
#define test_def(name) void _test_name(name)\
                            (struct _TestContext* _c)

void _test_append(const char* name, _TestFunction f) {
    struct _Test t = {
        .c.name = name,
        .f = f
    };
    da_append(_tests, t);
}

#define test_register(name) _test_append(#name, _test_name(name))
#define test_main(init) int main() {init;\
    size_t total = 0, error = 0;\
    for(size_t i = 0; i < _tests.size; ++i) {\
        struct _Test* t = &_tests.data[i];\
        t->f(&t->c);\
        total += t->c.total;\
        error += t->c.error;\
    }\
    if(error) {\
        fprintf(stderr, "failed: %zu, passed: %zu\n",\
        error, total-error);\
    }\
    return 0;\
}

//asserts
#define _assert(cond, text, ...) {++_c->total;\
    if(!(cond)) {\
        ++_c->error;\
        fprintf(stderr, BOLD_RED "[%s]: " text "\n" COLOR_RESET,\
        _c->name, __VA_ARGS__);\
    }\
}

#define assert_eq(a, b) _assert((a == b), #a " != " #b " (is %i)", a)
#define assert_sv_eq(a, b) _assert(sv_equal(&(a), b), #a " != " #b " (is \""sv_fmt "\")", sv_p(a))
#define assert_str_eq(a, b) _assert(strcmp(a, b) == 0, #a " != " #b " (is \"%s\")", a)
#define assert_sv_eqc(a, b, blen) _assert(sv_equalc(&(a), b, blen), #a " != " #b " (is \""sv_fmt "\")", sv_p(a))

#endif // CATE_TESTLIB_H