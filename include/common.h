#ifndef CATE_COMMON_H
#define CATE_COMMON_H
#include <stdio.h>

#if __STDC_VERSION__ < 202311L
    #define static_assert(cond, msg) _Static_assert(cond, msg)
    typedef uint8_t bool;
    enum {
        false = 0,
        true = 1,
    };
#endif

#define todo(text) {\
    fputs(text " is not implemented\n", stderr);\
    exit(1);\
}

//only use this with constant preprocessor strings
//sv_ccmp(v, str): BAD
//sv_ccmp(v, "str"): GOOD
#define sv_ccmp(sv, text) (sv_equalc(sv, text, sizeof(text)/sizeof(text[0])-1))

#endif // CATE_COMMON_H