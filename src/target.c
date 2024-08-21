#include "target.h"

CateOSTarget* cate_target = 
#ifdef _WIN32
&cate_target_windows
#else
&cate_target_posix
#endif // OS check
;

#ifdef __unix__
    #ifdef __linux__
        #define UNIX_PLATFORM_NAME "linux"
    #elif __FreeBSD__
        #define UNIX_PLATFORM_NAME "freebsd"
    #elif __NetBSD__
        #define UNIX_PLATFORM_NAME "netbsd"
    #elif __OpenBSD__
        #define UNIX_PLATFORM_NAME "openbsd"
    #elif __ANDROID__
        #define UNIX_PLATFORM_NAME "android"
    #else
        #error "what platform is this?"
    #endif
#endif
//for some reason macOS doesn't define __unix__?
#ifdef __APPLE__
    #define UNIX_PLATFORM_NAME "mac"
#endif

#define text(x) sv_from_const(x)

CateOSTarget cate_target_posix = {
    .dynamic_ending = text(".so"),
    .static_ending  = text(".a"),
    .object_ending  = text(".o"),
    .os_name_count  = 3
    #ifdef UNIX_PLATFORM_NAME
    +1
    #endif
    ,
    .os_names = {
    #ifdef UNIX_PLATFORM_NAME
        text(UNIX_PLATFORM_NAME),
    #endif
        text("posix"),
        text("unix"),
        text("bsd"),
    },
};

CateOSTarget cate_target_windows = {
    .dynamic_ending = text(".dll"),
    .static_ending  = text(".lib"),
    .object_ending  = text(".o"),
    .executable_ending = text(".exe"),
    .os_name_count = 3,
    .os_names = {
        text("windows"),
        text("win32"),
        text("win64"),
    },
};

uint8_t cate_platform_check(const string_view* id) {
    for (size_t i = 0; i < cate_target->os_name_count; ++i) {
        if(sv_equal(&cate_target->os_names[i], id))
            return 1;
    }
    return 0;
}