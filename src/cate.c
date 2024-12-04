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

/*---------------------.
| platform definitions |
`--------------------*/

//path translation is needed for non-unixes
void _translate_path(char** path);
#if !defined(__unix__) && !defined(__APPLE__)
#define translate_path(path) _translate_path(&path);
#else 
#define translate_path(path)
#endif

#define pstrlen(s) (sizeof(s)/(sizeof(s[0])))
#ifdef _WIN32
#define DYNAMIC_LIB_EXT ".dll"
#define STATIC_LIB_EXT ".lib"
#define OBJECT_FILE_EXT ".obj"
#define DIR_SEPARATOR "\\"

#else
#define DYNAMIC_LIB_EXT ".so"
#define STATIC_LIB_EXT ".a"
#define OBJECT_FILE_EXT ".o"
#define DIR_SEPARATOR "/"

#endif

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
    c_current_globals = g;
}

void cate_globals_free(CateGlobals* g) {
    free(g->std);
    free(g->build_dir);
    free(g->compiler);
    c_current_globals = 0;
}

/*---------------------.
| system (os specific) |
`--------------------*/
#ifdef WINDOWS
// void _translate_path(char** path) {

// }

#else
#include <unistd.h>
#include <sys/stat.h>
int cs_create_directory(char* dir) {
    if(cs_file_exists(dir)) return 1;
    return mkdir(dir, 0777) == 0;
}

int cs_file_exists(char* file) {
    return access(file, F_OK) == 0;
}

int cs_newer_than(char* file1, char* file2) {
    struct stat result;
    if(stat(file1, &result) != 0)
        return 0;
    size_t f1_time = result.st_mtime;

    if(stat(file2, &result) != 0)
        return 0;
    size_t f2_time = result.st_mtime;
    return f1_time > f2_time;
}
#endif