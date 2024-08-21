#include "catel.h"
#include "system_functions.h"
#include "target.h"
#include "error.h"
#include <errno.h>
#include <ctype.h>

static int catel_parse(CatelValues* c, string_view* text);
static void error(const char* fmt, ...);
void cate_help(int exit_code);

//TODO: Fix catel
void catel_build_path(struct CatePathBuilder* p, CatelValues* v,
                                string_view* file) {
    size_t dir_length = 0;
    if(v->dir[0]) {
        pb_from_cstr(p, v->dir);
        pb_append_dir_sep(p);
        dir_length = p->length;
    }
    pb_append_sv(p, file);
    if(!sv_ends_with(file, ".cate", 5))
        pb_append_cate_extension(p);

    if(cate_sys_file_exists(p->path.x)) return;

    //maybe it's in this dir?
    if(dir_length && cate_sys_file_exists(&p->path.x[dir_length])) {
        struct CateFullPath new = {0};
        memcpy(new.x, &p->path.x[dir_length], p->length-dir_length);
        memcpy(p->path.x, new.x, PATH_MAX);
    }
}

void catel_init(CatelValues* c) {
    //here we do have to null-terminate since it's paths.
    if(cate_sys_file_exists("cate")) {
        if(cate_sys_file_exists("cate/build.cate"))
            c->has_file = 1;
        strncpy(c->dir, "cate", 6);
    } else {
        if(cate_sys_file_exists("build.cate"))
            c->has_file = 1;
    }

    if(!cate_sys_file_exists(".catel"))
        return;

    string_view file = {0};
    int err = sv_load_file(&file, ".catel");
    if(err)
        error("can't open catel file because: %s", strerror(errno));

    if(!catel_parse(&catel, &file)) exit(-1);

    free(file.text);
}

static int catel_parse(CatelValues* c, string_view* text) {
    size_t i = 0;
    #define cur (text->text[i])
    #define match(k) (text->text[i].kind == k)
    #define while_line(cond) while (i < text->length && (cond))
    int changed = 0;
    char* prop = 0;
    while(i < text->length) {
        //skip whitespace
        while(isspace(cur)) ++i;

        //get the property or path
        size_t begin = i;
        while_line(!isspace(cur)) ++i;
        string_view v = sv_substring(text, begin, i);
        if(v.text[v.length] == '\0')
            v.length -= 1;

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

            memset(prop, 0, CATEL_PATH_SIZE);
            strncpy(prop, v.text, v.length);

            if(prop == c->file &&
                !sv_ends_with(&v, ".cate", 5)) {
                strncat(prop, ".cate", 6);
            }
            prop = 0;
            changed = 1;
        }
    }
    return changed;
}

static void error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    cate_error_va(fmt, args);
}