#ifndef CATE_ERROR_H
#define CATE_ERROR_H
#include <stdlib.h>
#include <stdarg.h>

void cate_error(const char* fmt, ...);
void cate_error_va(const char* fmt, va_list l);
void cate_warn(const char* text);
void cate_error_line(size_t line, const char* fmt, ...);
void cate_error_line_va(size_t line, const char* fmt, va_list l);
void cate_warn_line(size_t line, const char* text);

#define BOLD "\033[1m" 
#define COLOR_RESET "\033[0m"

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define PURPLE "\033[35m"
#define CYAN "\033[36m"
#define BOLD_RED "\033[1;31m"
#define BOLD_GREEN "\033[1;32m"
#define BOLD_PURPLE "\033[1;35m"
#define BOLD_CYAN "\033[1;36m"

#define list_color CYAN
#define hl_func(x) YELLOW x "()" COLOR_RESET
#define hl_func_params(x,...) YELLOW x "(" PURPLE __VA_ARGS__ YELLOW ")" COLOR_RESET
#define hl_var(x) PURPLE x COLOR_RESET
#define hl_flag(x) "\033[1;32m\t" x COLOR_RESET
#define traffic_light(good, sep, bad) GREEN good YELLOW sep RED bad COLOR_RESET
#define choose_light(good, sep, bad) BLUE good RED sep PURPLE bad COLOR_RESET
#define SUDO "\033[1;33msudo\033[1;34m "

#endif // CATE_ERROR_H