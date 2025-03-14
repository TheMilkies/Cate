#include "catel.h"
#include <ctype.h>

CatelFile catel = {
    .def.x = "build.cate",
    .dir.x = "."
};

enum {
    CATEL_TOK_DIR = TOK_COUNT_SIZE,
    CATEL_TOK_DEF,
};

void catel_run() {
    if(cs_file_exists("cate")) {
        memcpy(catel.dir.x, "cate", 5);
    }
    if(!cs_file_exists(".catel")) return;

    string_view file = {0};
    if(sv_load_file(&file, ".catel")) {
        cate_error("failed to open catel file!");
    }

    //parse
    #define cur file.text[i]
    #define next() ++i
    #define peek(n) file.text[i+n]
    #define in_range() (i < file.length)
    #define while_in(cond) while(in_range() && (cond))
    #define skip_until(ch) while_in(cur != ch) next();
    size_t i = 0;
    struct _CateFullPath* to_edit = 0;
    string_view property = {0}, value = {0};
    while (in_range()) {
        while_in(isspace(cur)) next();
        if(!cur) break;
        //parse property
        if(!property.length) {
            size_t begin = i;
            while_in(isalpha(cur)) next();
            property = sv_substring(&file, begin, i);

            if(sv_ccmp(&property, "dir") || sv_ccmp(&property, "directory")) {
                to_edit = &catel.dir;
            } else if(sv_ccmp(&property, "def")
                || sv_ccmp(&property, "default")) {
                to_edit = &catel.def;
            } else {
                cate_error("catel does not have \"" sv_fmt "\"\n"
                            "available are: def, default, dir, directory",
                    sv_p(property));
            }
            continue;
        }

        //get the value
        if(cur == '"') {
            next();
            size_t begin = i;
            while_in(cur != '"') next();
            if(!in_range()) {
                cate_error("unterminated string in catel!");
            }
            value = sv_substring(&file, begin, i);
            next();
        } else {
            size_t begin = i;
            while_in(!isspace(cur)) next();
            value = sv_substring(&file, begin, i);
        }

        if(property.length && value.length) {
            if(!to_edit) {
                cate_error("invalid catel state?");
            }
            if(value.length+1 >= FILENAME_MAX) {
                cate_error("catel value too long for this platform");
            }
            memcpy(to_edit->x, value.text, value.length);
            to_edit->x[value.length] = 0;

            //reset them for next pass
            property.length = 0;
            value.length = 0;
            to_edit = 0;
        }
    }
    #undef cur

    if(property.length) {
        cate_error("catel expects a value for \"" sv_fmt "\"",
                    sv_p(property));
    }
}