#include "catel.h"
#include "system_functions.h"
#include "target.h"
#include "error.h"
#include <errno.h>
#include <ctype.h>

static void catel_parse(CatelValues* c, string_view* text);
static void error(const char* fmt, ...);
void cate_help(int exit_code);

void catel_init(CatelValues* c) {
    if(cate_sys_file_exists("cate")) {
        strncpy(c->dir, "cate/", 6);
    } else {
        strncpy(c->dir, "./", 3);
    }

    if(!cate_sys_file_exists(".catel")) {
        if(cate_sys_file_exists("cate/build.cate")) {
            strncpy(c->dir, "cate/build.cate", 16);
            return;
        } else if(cate_sys_file_exists("build.cate")) {
            strncpy(c->dir, "build.cate", 11);
            return;
        }
        cate_error("no catefile provided, have some help");
        cate_help(1);
    }

    string_view file = {0};
    int err = sv_load_file(&file, ".catel");
    if(err)
        error("can't open catel file because: %s", strerror(errno));

    catel_parse(c, &file);

    free(file.text);
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
                !sv_ends_with(&(string_view) {.length = v.length, .text = prop},
                    ".cate", 5)) {
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
    exit(-1);
}