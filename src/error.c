#include "cate_error.h"
#include "target.h"
#include <stdio.h>

void cate_error(const char* fmt, ...) {
    fprintf(stderr, BOLD_RED"Error: " COLOR_RESET);
    va_list arg;
    va_start(arg, fmt);
    vfprintf(stderr, fmt, arg);
    fprintf(stderr, NL);
    exit(-1);
}

void cate_error_va(const char* fmt, va_list l) {
    fprintf(stderr, BOLD_RED"Error: "COLOR_RESET);
    vfprintf(stderr, fmt, l);
    fprintf(stderr, NL);
    exit(-1);
}

void cate_error_line_va(size_t line, const char* fmt, va_list l) {
    fprintf(stderr, BOLD_RED"Error in line %zu: "COLOR_RESET, line);
    vfprintf(stderr, fmt, l);
    fprintf(stderr, NL);
    exit(-1);
}

void cate_error_line(size_t line, const char* fmt, ...) {
    va_list arg;
    va_start(arg, fmt);
    cate_error_line_va(line, fmt, arg);
    exit(-1);
}

void cate_warn(const char* text) {
    fprintf(stderr, BOLD YELLOW "Warning: " COLOR_RESET "%s" NL, text);
}

void cate_warn_line(size_t line, const char* text) {
    fprintf(stderr, BOLD YELLOW "Warning in line %zu: "
        COLOR_RESET "%s" NL,
        line, text);
}
