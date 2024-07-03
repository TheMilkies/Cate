#ifndef CATE_TARGET_H
#define CATE_TARGET_H
#include <vendor/string_view.h>
#include <stdint.h>

typedef struct CateOSTarget {
    string_view dynamic_ending;
    string_view static_ending;
    size_t os_name_count;
    string_view os_names[];
} CateOSTarget;

extern CateOSTarget* cate_target;
extern CateOSTarget cate_target_windows;
extern CateOSTarget cate_target_posix;

uint8_t cate_platform_check(const string_view* id);

#ifdef _WIN32
#define NL "\r\n"
#else
#define NL "\n"
#endif // OS newline check

#endif // CATE_TARGET_H