#define _XOPEN_SOURCE 500
#include "cate.h"
#include <stdarg.h>

#define fatal(text) do{fprintf(stderr, "[cate] " text);\
    exit(-1);} while(0);
CateFlags c_cmd_flags = 0;

/*-------.
| memory |
`------*/
static void* xalloc(size_t n) {
    void* ptr = calloc(sizeof(uint8_t), n);
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
//commands don't hold their own strings, only free their data ptr.
typedef StringsArray Command;
static void cmd_free(Command* c);
static SysProc* cs_proc_create(Command* cmd);
static void cs_dry_run(Command* cmd);
static int cs_proc_exited(SysProc* proc);
static int cs_proc_get_exit_code(SysProc* proc);
static void cs_proc_kill(SysProc* proc);
static int cs_proc_wait(SysProc* proc);
static void cs_proc_free(SysProc* proc);
static void cg_get_thread_count();

/*--------.
| strings |
`-------*/
static size_t find_or_not(char* file, char c) {
    size_t size = strlen(file);
    for (intmax_t i = size; i > 0; --i) {
        if(file[i] == c) return i;
    }
    
    return size;
}

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

void cs_path_append(CateSysPath* p, char* text) {
    size_t len = strlen(text);
    if(p->length+len > FILENAME_MAX) {
        fatal("path too long for system");
    }

    memcpy(&p->x[p->length], text, len);
    p->length += len;
    p->x[p->length] = 0;
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
#define EXECUTABLE_INSTALL_LOCATION "C:\\cate\\programs\\"
#define LIBRARY_INSTALL_LOCATION "C:\\cate\\libraries\\"

#else
#define DYNAMIC_LIB_EXT ".so"
#define STATIC_LIB_EXT ".a"
#define OBJECT_FILE_EXT ".o"
#define DIR_SEPARATOR "/"

#define DEFAULT_COMPILER "cc"

#define EXECUTABLE_INSTALL_LOCATION "/usr/local/bin/"
#define LIBRARY_INSTALL_LOCATION "/usr/local/lib/"

#endif

#define SMOL_FLAGS_COUNT 7
#define SMOL_FLAGS \
    "-ffunction-sections", "-fdata-sections", "-Wl,--gc-sections",\
    "-fno-ident", "-fomit-frame-pointer", "-fmerge-all-constants",\
    "-Wl,--build-id=none"
//inlined ones crash so i'm doing it like this
static struct {
    char* out, *specify_link_script,
    *compile_objects, *load_library, *add_library_path,
    *add_include,
    *fpic, *shared, *debuggable,
    *archiver, *archiver_rcs,
    *smol_flags[SMOL_FLAGS_COUNT],
    *nul;
} program_options = {
    .out = "-o",
    .compile_objects = "-c",
    .load_library = "-l",
    .add_library_path = "-L",
    .add_include = "-I",
    .specify_link_script = "-T",
    .shared = "-shared",
    .fpic = "-fPIC",
    .debuggable = "-g",
    .archiver = "ar", .archiver_rcs = "rcs",
    .smol_flags = { SMOL_FLAGS }, 
    .nul = 0
};

/*--------.
| globals |
`-------*/
CateGlobals* c_current_globals = 0;
void c_globals_init(CateGlobals* g) {
    g->options = C_FLAGS_DEFAULT;
    g->compiler = c_string_clone(DEFAULT_COMPILER);
    g->linker = NULL;
    {
        char* x = (cs_file_exists("cate"))
                    ? "cate/build"
                    : "build";
        g->build_dir = c_string_clone(x);
    }
    c_current_globals = g;

    extern long cg_thread_count; //platform-defined!
    if(!cg_thread_count) cg_get_thread_count();
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
    c->options = c_current_globals->options;

    default(compiler);
    default(std);
    default(build_dir);
    default(linker);
    default(linker_script);

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
    free(c->build_dir);
    free(c->linker);
    free(c->linker_script);
    strings_free(&c->files);
    strings_free(&c->object_files);
    strings_free(&c->libraries);
    strings_free(&c->library_paths);
    strings_free(&c->flags);
    strings_free(&c->link_flags);
    strings_free(&c->includes);
}

typedef struct {
    char* src, *obj;
} BuildPair;
typedef da_type(BuildPair) BuildPairs;
static BuildPairs find_rebuildable(CateClass* c);

static char* make_out_name(CateClass* c);
static void objectify_files(CateClass* c);
static void make_command_template(CateClass* c, Command* cmd);
static Command make_build_command(Command* t, char* src, char* obj);
static int build_objects(CateClass* c, Command* tmp) {
    objectify_files(c);
    uint8_t needs_relink = 0;
    BuildPairs files = find_rebuildable(c);
    if(files.size || !cs_file_exists(c->out_name)) {
        needs_relink = 1;
    }

    make_command_template(c, tmp);
    for (size_t i = 0; i < files.size; ++i) {
        Command build = make_build_command(tmp,
            files.data[0].src,
            files.data[0].obj);
        SysProc *proc = cs_proc_create(&build);
        int code = cs_proc_wait(proc);
        if(code) {
            fatal("error in build command!\n");
        }
        cmd_free(&build);
    }
    free(files.data);
    return needs_relink;
}

static void class_automation(CateClass* c);
static int c_link(CateClass* c, Command* cmd);
void c_class_build(CateClass* c) {
    if(c->options & C_FLAG_AUTO) {
        class_automation(c);
    }
    Command tmp = {0};
    int needs_relink = build_objects(c, &tmp);

    if(c->options & C_FLAG_LINK && needs_relink) {
        c_link(c, &tmp);
    }
    cmd_free(&tmp);

    if(c->options & C_FLAG_SMOL) {
        cs_smolize(c->out_name);
    }
}

void c_class_clean(CateClass* c) {
    if(!c->object_files.size) {
        objectify_files(c);
    }

    for (size_t i = 0; i < c->object_files.size; ++i) {
        cs_remove_single(c->object_files.data[i]);
    }
}

static char* make_install_path(CateClass* c) {
    if(!c->out_name) c->out_name = make_out_name(c);
    size_t sep_position = find_or_not(c->out_name, DIR_SEPARATOR[0]);
    char* f = (c->out_name[sep_position] == DIR_SEPARATOR[0])
        ? &c->out_name[sep_position] : c->out_name;

    switch (c->kind) {
    case C_CLASS_PROJECT:
        return c_string_build(2, EXECUTABLE_INSTALL_LOCATION, f);
        break;

    case C_CLASS_LIB_STATIC:
    case C_CLASS_LIB_DYNAMIC:
        return c_string_build(2, LIBRARY_INSTALL_LOCATION, f);
        break;
    
    default:
        fatal("uninstallable class?");
        break;
    }
}

void c_class_install(CateClass* c) {
    char* path = make_install_path(c);
    //TODO: implement install
    free(path);
}

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

    if(!cs_create_directory(c->build_dir)) {
        fatal("failed to create build directory");
    }

    //create out directory
    {
        size_t sep_location = find_or_not(c->out_name, DIR_SEPARATOR[0]);
        if(c->out_name[sep_location] == DIR_SEPARATOR[0]) {
            char saved = c->out_name[sep_location];
            c->out_name[sep_location] = 0;
            if(!cs_create_directory(c->out_name)) {
                fatal("failed to create out directory");
            }
            c->out_name[sep_location] = saved;
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
        return c_string_build(3, "out/lib", c->name, DYNAMIC_LIB_EXT);
        break;

    case C_CLASS_LIB_STATIC:
        return c_string_build(3, "out/lib", c->name, STATIC_LIB_EXT);
        break;
    }

    fatal("invalid class type?");
    return NULL;
}

void c_add_file(CateClass* c, char* file) {
    char* t = c_string_clone(file);
    da_append(c->files, t);
}

void c_add_library(CateClass* c, char* name, int is_static) {
    
}

static char* objectify_file(char* file, char* build_dir) {
    /* unlike cate2, we do file.c.o instead of file.o
       for cases where there's disk.s and disk.c (oops) */
    //we don't use a fullpath for efficiency reasons
    const size_t build_dir_end = strlen(build_dir) + pstrlen(DIR_SEPARATOR) - 1;
    const size_t file_len = strlen(file);
    size_t len = build_dir_end + file_len + pstrlen(OBJECT_FILE_EXT) + 1;
    char* res = xalloc(len);

    //"$build_dir/"
    strcat(res, build_dir);
    strcat(res, DIR_SEPARATOR);

    //copy the filename there and convert all `/` to `_`
    for (size_t i = 0; i < file_len; ++i) {
        if(file[i] == DIR_SEPARATOR[0]) {
            res[build_dir_end+i] = '_';
        } else {
            res[build_dir_end+i] = file[i];
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

static void sa_append_no_copy(Command* dst, StringsArray* src) {
    if(!src) return;
    for (size_t i = 0; i < src->size; ++i) {
        char* p = src->data[i];
        da_append((*dst), p);
    }
}

static void cmd_append_prefixed(Command* dst, StringsArray* src,
                                char* flag) {
    if(!src) return;
    for (size_t i = 0; i < src->size; ++i) {
        da_append((*dst), flag);
        char* p = src->data[i];
        da_append((*dst), p);
    }
}

//we make a template that's shared between all classes.
static void make_command_template(CateClass* c, Command* cmd) {
    cmd->size = 0;
    //FIXME: std needs to be formed and freed
    da_append(*cmd, c->compiler);
    sa_append_no_copy(cmd, &c->flags);

    if(c->kind == C_CLASS_LIB_STATIC || c->kind == C_CLASS_LIB_DYNAMIC) {
        da_append(*cmd, program_options.shared);
        da_append(*cmd, program_options.fpic);
        da_append(*cmd, program_options.debuggable);
    }

    if(c->options & C_FLAG_SMOL) {
        for (size_t i = 0; i < SMOL_FLAGS_COUNT; ++i)
            da_append(*cmd, program_options.smol_flags[i]);
    }

    cmd_append_prefixed(cmd, &c->includes, program_options.add_include);
    cmd_append_prefixed(cmd,
            &c->library_paths, program_options.add_library_path);
    cmd_append_prefixed(cmd, &c->libraries, program_options.load_library);

    da_append(*cmd, program_options.compile_objects);
    da_append(*cmd, program_options.out);
}

static Command make_build_command(Command* t, char* src, char* obj) {
    Command c = {0};
    c.capacity = (t->size + 3) * sizeof(t->data[0]);
    c.data = xalloc(c.capacity);
    c.size = t->size;
    memcpy(c.data, t->data, t->size*sizeof(t->data[0]));
    da_append(c, obj);
    da_append(c, src);
    da_append(c, program_options.nul);

    return c;
}

static BuildPairs find_rebuildable(CateClass* c) {
    BuildPairs pairs = {0};
    for (size_t i = 0; i < c->files.size; ++i) {
        char* src = c->files.data[i],
            * obj = c->object_files.data[i];
        
        if(cs_newer_than(src, obj)) {
            BuildPair p = {.src = src, .obj = obj};
            da_append(pairs, p);
        }
    }

    return pairs;
}

static int c_link_generic(CateClass* c, Command* cmd) {
    //we add the -o first because ar wants it like that, others don't care
    //-o $name
    da_append(*cmd, program_options.out);
    da_append(*cmd, c->out_name);
    //-o $name $linkflags
    sa_append_no_copy(cmd, &c->link_flags);
    //-o $name $linkflags $objects
    sa_append_no_copy(cmd, &c->object_files);
    da_append(*cmd, program_options.nul);

    SysProc *proc = cs_proc_create(cmd);
    int exit = cs_proc_wait(proc);
    cs_proc_free(proc);
    return exit;
}

static int c_link_using_linker(CateClass* c, Command* cmd) {
    cmd->size = 0;
    da_append(*cmd, c->linker);
    cmd_append_prefixed(cmd,
            &c->library_paths, program_options.add_library_path);
    cmd_append_prefixed(cmd, &c->libraries, program_options.load_library);

    if(c->linker_script) {
        da_append(*cmd, program_options.specify_link_script);
        da_append(*cmd, c->linker_script);
    }
    return c_link_generic(c, cmd);
}

static int c_link(CateClass* c, Command* cmd) {
    if(c->linker) {
        return c_link_using_linker(c, cmd);
    }

    //pop the -c -o
    cmd->size -= 2;

    switch (c->kind) {
    case C_CLASS_PROJECT:
        return c_link_generic(c, cmd);
        break;

    case C_CLASS_LIB_DYNAMIC:
        da_append(*cmd, program_options.shared);
        da_append(*cmd, program_options.debuggable);
        return c_link_generic(c, cmd);
        break;

    case C_CLASS_LIB_STATIC:
        cmd->size = 0;
        da_append(*cmd, program_options.archiver);
        da_append(*cmd, program_options.archiver_rcs);
        return c_link_generic(c, cmd);
        break;
    
    default:
        fatal("unlinkable class type");
        break;
    }

    return 1;
}

static void cs_dry_run(Command* cmd) {
    for (size_t i = 0; i < cmd->size-1; ++i) {
        printf("%s ", cmd->data[i]);
    }
    printf("\n");
}

static void cmd_free(Command* c) {
    free(c->data);
}

/*---------------------.
| system (os specific) |
`--------------------*/
#ifdef _WIN32
// void _translate_path(char** path) {

// }

#elif __unix__
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>
#include <dirent.h>
#include <ftw.h>

#ifdef _SC_NPROCESSORS_ONLN
long cg_thread_count = 0;
static void cg_get_thread_count() {
    cg_thread_count = sysconf(_SC_NPROCESSORS_ONLN);
}
#else
long cg_thread_count = 1;
//should NEVER be called. just here to remove the error.
static void cg_get_thread_count() {return;}
#endif

#ifdef __linux__
#include <sys/sendfile.h>
#else
//damn it BSD!
static ssize_t sendfile(int out, int in, off_t* offset, size_t size) {
    char buf[4096] = {0};
    ssize_t nread, total = 0;
    off_t offset_remaining = *offset;

    while (total <= size) {
        if (offset_remaining > 0) {
            off_t curr_offset = lseek(in, offset_remaining, SEEK_SET);
            if (curr_offset == -1)
                return -1;

            offset_remaining = 0;
        }

        nread = read(in, buf, sizeof(buf));
        if (nread <= 0)
            break;

        ssize_t nwritten = write(out, buf, nread);
        if (nwritten <= 0)
            return -1;

        total += nwritten;
    }

    *offset = offset_remaining + total;
    return total;
}
#endif

static int _open(const char *path, int flags, int opt) {
    int f = open(path, flags, opt);
    if(f < 0) {
        fprintf(stderr, "[cate] failed to open file \"%s\"", path);
        exit(-1);
    }
    return f;
}

int cs_copy(char* path1, char* path2) {
    if(c_cmd_flags & C_CMD_DRY_RUN) {
        printf("cp %s %s\n", path1, path2);
        return 1;
    }
    int result = 1;

    int in =  _open(path1, O_RDONLY, 0);
    struct stat st;
    if(fstat(in, &st) == -1) {
        result = 0;
        goto bad;
    }
    int out = _open(path2, O_WRONLY | O_CREAT | O_TRUNC, st.st_mode);

    //i ported sendfile because it's the easiest API... here
    off_t offset = 0;
    int sent = sendfile(out, in, &offset, st.st_size);
    if(sent == -1)
        result = 0;

bad:
    close(in);
    close(out);
    return result;
}

static int _mkdir(const char* path) {
    return !(mkdir(path, S_IRWXU) && errno != EEXIST);
}

static inline int _recursive_mkdir(const char *dir) {
    char tmp[FILENAME_MAX] = {0};
    char *p = 0;
    size_t len = 0;

    snprintf(tmp, sizeof(tmp),"%s",dir);
    len = strlen(tmp);
    if (tmp[len - 1] == '/')
        tmp[len - 1] = 0;
    for (p = tmp + 1; *p; p++)
        if (*p == '/') {
            *p = 0;
            if(!_mkdir(tmp)) return 0;
            *p = '/';
        }
    return _mkdir(tmp);
}

int cs_create_directory(char* dir) {
    if(cs_file_exists(dir)) return 1;
    if(c_cmd_flags & C_CMD_DRY_RUN) {
        printf("mkdir -p %s\n", dir);
        return 1;
    }
    return _recursive_mkdir(dir);
}

int cs_move(char* file1, char* file2) {
    if(c_cmd_flags & C_CMD_DRY_RUN) {
        printf("mv %s %s\n", file1, file2);
        return 1;
    }
    return rename(file1, file2) == 0;
}

int cs_remove_single(const char* file) {
    if(c_cmd_flags & C_CMD_DRY_RUN) {
        printf("rm -f %s\n", file);
        return 1;
    }

    int err = remove(file);
    if(err) {
        fprintf(stderr, "[cate] failed to remove \"%s\" because: \n", file,
            strerror(errno));
        return 1;
    }

    return err;
}

int _rm_callback(const char *fpath, const struct stat *sb, int typeflag,
                 struct FTW *ftwbuf) {
    
    return cs_remove_single(fpath);
}

int cs_remove(char* file) {
    if(c_cmd_flags & C_CMD_DRY_RUN) {
        printf("rm -rf %s\n", file);
        return 1;
    }
    
    return nftw(file, _rm_callback, 64, FTW_DEPTH | FTW_PHYS) == 0;
}

int cs_file_exists(char* file) {
    return access(file, F_OK) == 0;
}

int cs_newer_than(char* file1, char* file2) {
    struct stat result;
    if(stat(file1, &result) != 0)
        return 1;
    size_t f1_time = result.st_mtime;

    if(stat(file2, &result) != 0)
        return 1;
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

static SysProc* cs_proc_create(Command* cmd) {
    if(c_cmd_flags & C_CMD_DRY_RUN) {
        cs_dry_run(cmd);
        return 0;
    }

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
    if(!proc) return 1; // for dry runs
    if(waitpid(proc->pid, &proc->status, WNOHANG) == -1) {
        fatal("failed to get process status?");
    }
    return WIFEXITED(proc->status);
}

static int cs_proc_wait(SysProc* proc) {
    if(!proc) return 0; // for dry runs
    if(waitpid(proc->pid, &proc->status, 0) == -1) {
        fatal("failed to get process status?");
    }
    return WEXITSTATUS(proc->status);
}

static void cs_proc_free(SysProc* proc) {
    free(proc); //null is ignored (dry run)
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
#error "Cate doesn't support this platform."

#endif

void cs_path_directory_separator(CateSysPath* p) {
    cs_path_append(p, DIR_SEPARATOR);
}
