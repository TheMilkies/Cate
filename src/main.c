#include "cmd_args.h"
#include "target.h"
#include "error.h"
#include "common.h"
#include <vendor/dynamic_array.h>
#include <ctype.h>

#define CATE_VERSION "3.0"
CmdArgs cmd_args = {0};

static char *shift_args(int *argc, char ***argv) {
    if(*argc <= 0) return 0;
    char *result = **argv;
    *argc -= 1;
    *argv += 1;
    return result;
}
#define shift_args() shift_args(&argc, &argv)

static int init_project(const char* const name);
static void cate_help(int exit_code) {
    puts(BOLD CYAN
        "Cate v" CATE_VERSION " by TheMilkies and Ayinsonu." NL
        "usage: "COLOR_RESET"cate " BOLD GREEN "[flags] " "[filename]" NL
        NL
        "flags:\n"
        ""
        //TODO: Add flags here
    );
    exit(exit_code);
}

int main(int argc, char *argv[]) {
    shift_args(); //skip the program name
    //preallocate 128kb so malloc won't call sbrk constantly
    free(malloc(128*1024));
    da_type(char*) files = {0};
    while (argc > 0) {
        char* arg  = shift_args();
        if(arg[0] != '-') {
            da_append(files, arg);
            continue;
        }

        switch (arg[1]) {
        case 'v':
            puts("v"CATE_VERSION);
            return 0;
            break;
        case 'h':
        case '?':
            cate_help(0);
            break;
        
    #define flag(ch, does)\
        case ch:\
            cmd_args.flags |= CMD_ ## does;\
            break;
        
        case 'B': flag('f', FORCE_REBUILD)

        flag('S', FORCE_SMOLIZE)
        flag('D', DISABLE_SYSTEM)
        flag('d', DRY_RUN)
        flag('y', ALWAYS_INSTALL)
        flag('n', NEVER_INSTALL)

    #define get_val(kind)\
        if(arg[2]) val = &arg[2];\
        else if(argc != 0) val = shift_args();\
        else { \
            cate_error("expected a "kind" value for the -%c flag",\
                arg[1]);\
            return 1;\
        }\

        case 'j': // for make support
        case 't': {
            char* val = 0;
            get_val("numeric")
            char* strerr = 0;
            cmd_args.thread_count = strtol(val, &strerr, 10);
            if(strerr == val || cmd_args.thread_count <= 0) {
                cate_error("invalid value for the -%c flag",
                    arg[1]);
                return 1;
            }
        } break;

        case 'i': {
            const char* val = 0;
            get_val("identifier")
            if(!val[0] || (!isalpha(val[0]) && val[0] != '_')) {
                cate_error("invalid identifier for the new project's name",
                    arg[1]);
                return 1;
            }
            return init_project(val);
        } break;

        case 'l': {
            todo("the -l flag");
        } break;
        
        default:
            cate_error("invalid flag: \"%s\", use `cate -h` "
                "to see which args are supported",
                arg);
                return 1;
            break;
        }
    }

    if(files.size == 0) {
        cate_error("no file given and catel is not implemented yet");
        return 1;
    }

    return 0;
}

static int init_project(const char* const name) {
    todo("init_project()");
    return 0;
}