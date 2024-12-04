#include "cate.h"

/*--------.
| strings |
`-------*/
char* cate_string_clone(char* s) {
    return strdup(s);
}

char* cate_string_build(int count, ...) {
    size_t max = 0;
    va_list args;
    va_start(args, count);

    for (size_t i = 0; i < count; ++i) {
        char* s = va_arg(args, char*);
        max += strlen(s);
    }
    
    va_start(args, count);
    char* res = calloc(max+1, sizeof(char));
    for (size_t i = 0; i < count; ++i) {
        char* s = va_arg(args, char*);
        strcat(res, s);
    }

    va_end(args);
    return res;
}

static void strings_free(StringsArray* a) {
    for (size_t i = 0; i < a->size; i++)
        free(a->data[i]);
    
    free(a->data);
}

char* sv_clone_as_cstr(string_view* v) {
    char* s = malloc(v->length+1);
    memcpy(s, v->text, v->length);
    s[v->length+1] = '\0';
    return s;
}

/*--------.
| globals |
`-------*/
CateGlobals* c_current_globals = 0;
void cate_globals_init(CateGlobals* g) {
    g->options = CATE_FLAGS_DEFAULT;
    g->compiler = string_clone("cc");
    {
        static char* cate_dir = "cate/build";
        static char* build_dir = "build";
        char* x = (cs_file_exists(cate_dir))
                        ? cate_dir
                        : build_dir;
        g->build_dir = string_clone(x);
    }
}

void cate_globals_free(CateGlobals* g) {

}


