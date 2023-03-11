#pragma once
#define CATE_VERSION "v2.9"

#define BOLD "\033[1m" 
#define COLOR_RESET "\e[0m"

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define PURPLE "\033[35m"
#define CYAN "\033[36m"

#define hl_func(x) YELLOW x COLOR_RESET
#define hl_var(x) PURPLE x COLOR_RESET
#define hl_flag(x) "\e[1;32m\t" x COLOR_RESET