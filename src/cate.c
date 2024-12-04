#include "cate.h"
#include <stdarg.h>

#define fatal(text) do{fprintf(stderr, "[cate] " text);\
    exit(-1);} while(0);

/*-------.
| memory |
`------*/
static void* xalloc(size_t n) {
    void* ptr = calloc(1, n);
    if(!ptr) {
        fatal("out of memory!");
    }
    return ptr;
}

/*----------.
| processes |
`---------*/
struct SysProc;
typedef struct SysProc SysProc;
static SysProc* cs_proc_create(StringsArray* cmd);
static int cs_proc_exited(SysProc* proc);
static int cs_proc_get_exit_code(SysProc* proc);
static void cs_proc_kill(SysProc* proc);
static int cs_proc_wait(SysProc* proc);

/*--------.
| strings |
`-------*/
char* c_string_clone(char* s) {
    size_t len = strlen(s);
    char* r = xalloc(len+1);
    memcpy(r, s, len);
    return r;
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
    char* res = xalloc((max+1)*sizeof(char));
    for (size_t i = 0; i < count; ++i) {
        char* s = va_arg(args, char*);
        strcat(res, s);
    }

    va_end(args);
    return res;
}

static void strings_free(StringsArray* a) {
    for (size_t i = 0; i < a->size; ++i)
        free(a->data[i]);
    
    free(a->data);
}

char* sv_clone_as_cstr(string_view* v) {
    char* s = xalloc(v->length+1);
    memcpy(s, v->text, v->length);
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
    strings_free(&c->files);
    strings_free(&c->object_files);
    strings_free(&c->libraries);
    strings_free(&c->library_paths);
    strings_free(&c->flags);
    strings_free(&c->link_flags);
    strings_free(&c->includes);
}

static void objectify_files(CateClass* c);
static void class_automation(CateClass* c);
void c_class_build(CateClass* c) {
    if(c->options & C_FLAG_AUTO) {
        class_automation(c);
    }

    objectify_files(c);
}

static size_t find_or_not(char* file, char c) {
    size_t size = strlen(file);
    for (intmax_t i = size; i > 0; --i) {
        if(file[i] == c) return i;
    }
    
    return size;
}

static char* make_out_name(CateClass* c);
static void class_automation(CateClass* c) {
    if(!c->out_name) {
        c->out_name = make_out_name(c);
    }

    if(!c->includes.size) {
        static char* inc = "include";
        if(cs_file_exists(inc)) {
            char* x = c_string_clone(inc);
            da_append(c->includes, x);
        }
    }

    c_clone_from_global(c);
}

static char* make_out_name(CateClass* c) {
    switch (c->kind) {
    case C_CLASS_PROJECT:
    #ifdef WINDOWS
        return c_string_build(2, c->name, ".exe");
    #else
        return c_string_clone(c->name);
    #endif
        break;
    case C_CLASS_LIB_DYNAMIC:
        return c_string_build(3, "lib", c->name, DYNAMIC_LIB_EXT);
        break;

    case C_CLASS_LIB_STATIC:
        return c_string_build(3, "lib", c->name, STATIC_LIB_EXT);
        break;
    }

    fatal("invalid class type?");
    return NULL;
}

void c_add_file(CateClass* c, char* file) {
    char* t = c_string_clone(file);
    da_append(c->files, t);
}

static char* objectify_file(char* file, char* build_dir) {
    size_t dot_location = find_or_not(file, '.');
    size_t len = strlen(build_dir) + pstrlen(DIR_SEPARATOR) - 1;
    size_t max = len + dot_location + pstrlen(OBJECT_FILE_EXT);
    char* res = xalloc(max+sizeof(char));

    //"$build_dir/"
    strcat(res, build_dir);
    strcat(res, DIR_SEPARATOR);

    //convert all `/` to `_`
    for (size_t i = 0; i < dot_location; ++i) {
        if(file[i] == DIR_SEPARATOR[0]) {
            res[len+i] = '_';
        } else {
            res[len+i] = file[i];
        }
    }

    //"$build_dir/$file.o"
    strncat(res, OBJECT_FILE_EXT, pstrlen(OBJECT_FILE_EXT));

    return res;
}

static void objectify_files(CateClass* c) {
    c->object_files.size = 0;
    for (size_t i = 0; i < c->files.size; ++i) {
        char* s = objectify_file(c->files.data[i], c->build_dir);
        da_append(c->object_files, s);
    }
}

/*---------------------.
| system (os specific) |
`--------------------*/
#ifdef _WIN32
// void _translate_path(char** path) {

// }

#elif __unix__
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

#include <sys/wait.h>
#include <signal.h>
/*----------.
| processes |
`---------*/
struct SysProc {
    pid_t pid;
    int status;
};

static SysProc* cs_proc_create(StringsArray* cmd) {
    SysProc* p = xalloc(sizeof(*p));
    p->pid = fork();
    if(p->pid == 0) {
        execvp(cmd->data[0], cmd->data);
        fprintf(stderr, "[cate] %s: program not found!\n",
                    cmd->data[0]);
        exit(2);
    } else if (p->pid < 0) {
        fatal("failed to create subprocess!");
    }

    return p;
}

static int cs_proc_exited(SysProc* proc) {
    if(waitpid(proc->pid, &proc->status, WNOHANG) == -1) {
        fatal("failed to get process status?");
    }
    return WIFEXITED(proc->status);
}

static int cs_proc_wait(SysProc* proc) {
    if(waitpid(proc->pid, &proc->status, 0) == -1) {
        fatal("failed to get process status?");
    }
    return WEXITSTATUS(proc->status);
}

static int cs_proc_get_exit_code(SysProc* proc) {
    if(!WIFEXITED(proc->status))
        fatal("this is a bug #0");
    return WEXITSTATUS(proc->status);
}

static void cs_proc_kill(SysProc* proc) {
    kill(proc->pid, SIGKILL);
}

int cs_smolize(char* file) {
    return 1;
}

#else
#error "Cate3 doesn't support this OS."

#endif