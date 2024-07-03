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

#endif // CATE_COMMON_H