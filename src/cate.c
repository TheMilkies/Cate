#include "cate.h"
#include <stdarg.h>

/*--------.
| strings |
`-------*/
char* c_string_clone(char* s) {
    return strdup(s);
}

char* c_string_build(int count, ...) {
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

#define DEFAULT_COMPILER "cc"
#define DEFAULT_LINKER "ld"

#else
#define DYNAMIC_LIB_EXT ".so"
#define STATIC_LIB_EXT ".a"
#define OBJECT_FILE_EXT ".o"
#define DIR_SEPARATOR "/"

#define DEFAULT_COMPILER "cc"
#define DEFAULT_LINKER "ld"

#endif

/*--------.
| globals |
`-------*/
CateGlobals* c_current_globals = 0;
void c_globals_init(CateGlobals* g) {
    g->options = C_FLAGS_DEFAULT;
    g->compiler = c_string_clone(DEFAULT_COMPILER);
    g->linker = c_string_clone(DEFAULT_LINKER);
    {
        static char* cate_dir = "cate/build";
        static char* build_dir = "build";
        char* x = (cs_file_exists(cate_dir))
                        ? cate_dir
                        : build_dir;
        g->build_dir = c_string_clone(x);
    }
    c_current_globals = g;
}

void c_globals_free(CateGlobals* g) {
    free(g->compiler);
    free(g->build_dir);
    free(g->std);
    free(g->linker);
    c_current_globals = 0;
}

/*--------.
| classes |
`-------*/
static void c_clone_from_global(CateClass* c) {
#define default(prop) if(!c->prop && c_current_globals->prop)\
    c->prop = c_string_clone(c_current_globals->prop);

    default(compiler);
    default(linker);
    default(std);
    default(build_dir);

#undef default
}

CateClass c_class(char* name, CateClassKind kind) {
    CateClass c = {.kind = kind};
    c.name = c_string_clone(name);
    c_clone_from_global(&c);
    return c;
}

void c_class_free(CateClass* c) {
    free(c->name);
    free(c->out_name);
    free(c->compiler);
    free(c->std);
    free(c->linker);
    free(c->build_dir);
    strings_free(&c->libraries);
    strings_free(&c->library_paths);
    strings_free(&c->flags);
    strings_free(&c->link_flags);
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