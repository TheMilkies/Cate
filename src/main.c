#include "cmd_args.h"
#include "parser.h"
#include "error.h"
#include "common.h"
#include "catel.h"
#include "system_functions.h"
#include <vendor/dynamic_array.h>
#include <ctype.h>

#define CATE_VERSION "3.0.0"
CmdArgs cmd_args = {0};
CateContext ctx = {0};

static char *shift_args(int *argc, char ***argv) {
    if(*argc <= 0) return 0;
    char *result = **argv;
    *argc -= 1;
    *argv += 1;
    return result;
}
#define shift_args() shift_args(&argc, &argv)

static int init_project(const char* const name);
void cate_help(int exit_code) {
    puts(BOLD_CYAN
    "Cate v" CATE_VERSION " by TheMilkies and " PURPLE"Ayin" YELLOW"Sonu." NL
    CYAN "usage: "COLOR_RESET"cate " BOLD GREEN "[flags] " PURPLE "[filename]" NL
    NL
    BOLD GREEN "flags:" NL
    hl_flag("-l") ":  list all catefiles in default directory" NL
    "\t" BOLD GREEN "-i" hl_var("V") ": init a project with the name " PURPLE BOLD "V" NL
	hl_flag("-f") ":  force rebuild" NL
    hl_flag("-d") ":  print all commands in script without running them. (dry run)" NL
    hl_flag("-y") ":  install without asking (always answer 'y')" NL
	hl_flag("-n") ":  never install (always answer 'n')" NL
	hl_flag("-D") ":  disable all " hl_func("system") " calls in script" NL
    "\t" BOLD GREEN "-t" hl_var("N") ": set thread count to " BOLD_PURPLE "N" NL
	hl_flag("-S") ":  smolize even if not set in script" NL
	hl_flag("-v") ":  show version" NL
	hl_flag("-h") ":  show help (this)"
    );
    exit(exit_code);
}

int main(int argc, char *argv[]) {
    cmd_args.thread_count = cate_sys_get_core_count();
    shift_args(); //skip the program name
    //128KB is overkill for most catefiles but it speeds up everything
    st_init(&ctx.st, 128*1024);
    //avoid calling sbrk() a lot
    free(malloc(sizeof(STIndex)*10*10));

    //handle catel (god damn it milkies!)
    static CatelValues defaults = {0};
    catel_init(&defaults);

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

        case 'A':
            cate_warn("the -A flag is deprecated.");
            break;

    #define get_val(kind)\
        if(arg[2]) val = &arg[2];\
        else if(argc != 0) val = shift_args();\
        else { \
            cate_error("expected a "kind" value for the -%c flag",\
                arg[1]);\
            return 1;\
        }\

        case 'j': // for the make users that never learn
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
            struct CateSysDirectory* d = cate_sys_open_dir(defaults.dir);
            if(!d) {
                cate_error("can't open default directory!");
                return 1;
            }

            //TODO: highlight default file
            struct CateSysDirEntry ent = {0};
            printf(list_color);
            while (cate_sys_dir_get(d, &ent)) {
                string_view name = sv_from_cstr(ent.name);
                if(sv_ends_with(&name, ".cate", 5)) {
                    printf(sv_fmt" ", name.length-5, name.text);
                }
            }
            printf(COLOR_RESET);
            cate_sys_dir_close(d);
            return 0;
        } break;
        
        default:
            cate_error("invalid flag: \"%s\", use `cate -h` "
                "to see which args are supported",
                arg);
                return 1;
            break;
        }
    }

    //both -y and -n can't be active at the same time
    if(cmd_args.flags & CMD_ALWAYS_INSTALL
    && cmd_args.flags & CMD_NEVER_INSTALL) {
        cate_warn("both -y and -n are used, will ask");
        //disable both
        cmd_args.flags &= ~(CMD_ALWAYS_INSTALL | CMD_NEVER_INSTALL);
    }

    if(files.size == 0) {
        cate_error("no file given and catel is not implemented yet");
        return 1;
    }

    for (size_t i = 0; i < files.size; ++i) {
        cate_open(files.data[i]);
        context_reset();
    }

    da_free(files);
    context_free();
    return 0;
}

static FILE* create_file(const char* name) {
    FILE* f = fopen(name, "wb");
    if(!f)
        cate_error("can't create file \"%s\"", name);
    return f;
}

static int init_project(const char* const name) {
    if(cate_sys_file_exists("src/main.c") && cate_sys_file_exists(".catel")) {
        cate_error("project already inited");
        return 1;
    }

    cate_sys_mkdir("cate");
    cate_sys_mkdir("src");
    cate_sys_mkdir("include");
    FILE* src = create_file("src/main.c");
    FILE* build = create_file("cate/build.cate");
    FILE* debug = create_file("cate/debug.cate");
    FILE* catel = create_file(".catel");

    //src/main.c
    fprintf(src,
        "#include <stdio.h>" NL
        "#include <stdlib.h>" NL
        NL
        "int main(int argc, char* argv[]) {" NL
            "\tputs(\"Hello, World!\");" NL
            "\treturn 0;" NL
        "}"
    );
    fclose(src);

    //catel
    fprintf(catel, "def debug.cate");
    fclose(catel);

    //build.cate, debug.cate
    fprintf(debug, 
        "Project %s" NL
        ".flags = \"-ggdb3\"" NL , name);
    fprintf(build,
        "smol = true" NL
        "Project %s" NL
        ".flags = \"-O3\"" NL, name);

    static const char* shared =
        ".files = {recursive(\"src/*.c\")}" NL
        ".includes = {\"include\"}" NL
        ".build()";
    fputs(shared, debug);
    fputs(shared, build);

    fclose(debug);
    fclose(build);

    return 0;
}