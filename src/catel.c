#include "catel.h"
#include "common.h"
#include "system_functions.h"
#include "target.h"
#include "error.h"
#include <errno.h>
#include <ctype.h>

static void catel_parse(string_view* text);
static void error(const char* fmt, ...);
void cate_help(int exit_code);

//this returns a char* for efficiency
//we could memcpy but that's really wasteful
char* catel_build_path(struct CatePathBuilder* p,
                                char* file) {
    size_t dir_length = 0;
    if(catel.dir[0]) {
        pb_from_cstr(p, catel.dir);
        pb_append_dir_sep(p);
        dir_length = p->length;
    }
    string_view file_as_sv = sv_from_cstr(file);
    pb_append_sv(p, &file_as_sv);
    if(!sv_ends_with(&file_as_sv, ".cate", 5))
        pb_append_cate_extension(p);

    if(cate_sys_file_exists(p->path.x))
        return p->path.x;

    //maybe it's in this dir?
    if(dir_length && cate_sys_file_exists(&p->path.x[dir_length]))
        return &p->path.x[dir_length];
    return p->path.x;
}

void catel_init() {
    //here we do have to null-terminate since it's paths.
    if(cate_sys_file_exists("cate")) {
        if(cate_sys_file_exists("cate/build.cate"))
            catel.has_file = 1;
        strncpy(catel.dir, "cate", 6);
    } else {
        if(cate_sys_file_exists("build.cate"))
            catel.has_file = 1;
    }

    if(!cate_sys_file_exists(".catel"))
        return;

    string_view file = {0};
    int err = sv_load_file(&file, ".catel");
    if(err)
        error("can't open catel file because: %s", strerror(errno));

    catel_parse(&file);

    free(file.text);
}

static void catel_parse(string_view* text) {
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

        if(i >= text->length)
            v.length -= 1;

        if(!prop) {
            if(sv_ccmp(&v, "def") || sv_ccmp(&v, "default")) {
                prop = catel.file;
            } else if(sv_ccmp(&v, "dir") || sv_ccmp(&v, "directory")) {
                prop = catel.dir;
            } else {
                error("catel doesn't support property \""sv_fmt"\"", sv_p(v));
            }
        } else {
            if(v.length >= CATEL_PATH_MAX)
                error("path \""sv_fmt"\" is too long", sv_p(v));

            memset(prop, 0, CATEL_PATH_SIZE);
            strncpy(prop, v.text, v.length);

            if(prop == catel.file &&
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