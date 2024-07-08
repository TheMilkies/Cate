#include "catel.h"
#include "system_functions.h"
#include "target.h"
#include "error.h"
#include <errno.h>
#include <ctype.h>

static void catel_parse(CatelValues* c, string_view* text);
static void error(const char* fmt, ...);
void cate_help(int exit_code);

//BROKEN, DO NOT USE
//TODO: Fix catel
void catel_init(CatelValues* c) {
    strncpy(c->file, "build.cate", 11);

    if(cate_sys_file_exists("cate")) {
        if(cate_sys_file_exists("cate/build.cate")) {
            strncpy(c->file_path, "cate/build.cate", 16);
        }
        strncpy(c->dir, "cate/", 6);
    } else {
        if(cate_sys_file_exists("build.cate")) {
            strncpy(c->file_path, "build.cate", 11);
        }
        strncpy(c->dir, "./", 3);
    }

    if(!cate_sys_file_exists(".catel")) {
        return;
    }

    // string_view file = {0};
    // int err = sv_load_file(&file, ".catel");
    // if(err)
    //     error("can't open catel file because: %s", strerror(errno));

    // catel_parse(c, &file);

    // if(c->file_path[0])
    //     memset(c->file_path, 0, PATH_MAX);
    // //build the string, the cate extension is added by the parser
    // {
    //     const size_t dir_length = strlen(c->dir);
    //     const size_t name_length = strlen(c->file);

    //     const size_t length = dir_length + 1 + name_length;
    //     if(length >= PATH_MAX)
    //         error("catel-created path is too long");

    //     strncpy(c->file_path, c->dir, dir_length);
    //     strncpy(c->file_path, "/", 2);
    //     strncpy(c->file_path, c->file, name_length);
    // }

    // free(file.text);
}

static void catel_parse(CatelValues* c, string_view* text) {
    size_t i = 0;
    #define cur (text->text[i])
    #define match(k) (text->text[i].kind == k)
    #define while_line(cond) while (i < text->length && (cond))

    char* prop = 0;
    while(i < text->length) {
        //skip whitespace
        while(isspace(cur)) ++i;

        //get the property or path
        size_t begin = i;
        while_line(!isspace(cur)) ++i;
        string_view v = sv_substring(text, begin, i);

        if(!prop) {
            if(sv_equalc(&v, "def", 3) || sv_equalc(&v, "default", 7)) {
                prop = c->file;
            } else if(sv_equalc(&v, "dir", 3)
            || sv_equalc(&v, "directory", 9)) {
                prop = c->dir;
            } else {
                error("catel doesn't support property \""sv_fmt"\"", sv_p(v));
            }
        } else {
            if(v.length >= CATEL_PATH_MAX)
                error("path \""sv_fmt"\" is too long", sv_p(v));

            strncpy(prop, v.text, v.length);

            if(prop == c->file &&
                !sv_ends_with(&v, ".cate", 5)) {
                strncat(prop, ".cate", 6);
            }
            prop = 0;
        }
    }
}

static void error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    cate_error_va(fmt, args);
}