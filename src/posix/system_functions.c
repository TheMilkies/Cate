#include "system_functions.h"
#include "cmd_args.h"
#include "error.h"
#include "target.h"
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <assert.h>
#include <dirent.h>

static inline int get_exit_code(int status);

size_t cate_sys_get_core_count() {
    return sysconf(_SC_NPROCESSORS_ONLN)*2;
}

int cate_sys_file_exists(const char* path) {
    return access(path, F_OK) == 0;
}

static int _mkdir(const char* path) {
    return !(mkdir(path, S_IRWXU) && errno != EEXIST);
}

static inline int _recursive_mkdir(const char *dir) {
    char tmp[PATH_MAX] = {0};
    char *p = 0;
    size_t len = 0;

    snprintf(tmp, sizeof(tmp),"%s",dir);
    len = strlen(tmp);
    if (tmp[len - 1] == '/')
        tmp[len - 1] = 0;
    for (p = tmp + 1; *p; p++)
        if (*p == '/') {
            *p = 0;
            if(_mkdir(tmp) != 1) return 0;
            *p = '/';
        }
    return _mkdir(tmp) == 1;
}

int cate_sys_mkdir(char* path) {
    if(cate_sys_file_exists(path)) return 1;
    if(cmd_args.flags & CMD_DRY_RUN) {
        printf("mkdir -p %s\n", path);
        return 1;
    }
    return _recursive_mkdir(path);
}

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
        cate_error("failed to open file \"%s\"");
    }
    return f;
}

int cate_sys_copy(char* path1, char* path2) {
    if(cmd_args.flags & CMD_DRY_RUN) {
        printf("cp %s %s\n", path1, path2);
        return 1;
    }
    int result = 1;

    int in =  _open(path1, O_RDONLY, 0);
    int out = _open(path2, O_WRONLY | O_CREAT | O_TRUNC, 0666);

    struct stat st;
    if(fstat(in, &st) == -1) {
        result = 0;
        goto bad;
    }

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

int cate_sys_move(char* path1, char* path2) {
    if(cmd_args.flags & CMD_DRY_RUN) {
        printf("mv %s %s\n", path1, path2);
        return 1;
    }
    return rename(path1, path2) == 0;
}

int cate_sys_system(const char* cmd) {
    if(cmd_args.flags & CMD_DRY_RUN) {
        printf("sh \"%s\"\n", cmd);
        return 0;
    }
    int status = system(cmd);
    return get_exit_code(status);
}

size_t cate_get_modified_time(const char* path) {
    struct stat attr;
    if (stat(path, &attr) != 0) //check if file exists
        return 0; //will always recompile since object file doesn't exist
    return attr.st_mtime;
}

int cate_is_file_newer(const char* path1, const char* path2) {
    return cate_get_modified_time(path1) > cate_get_modified_time(path2);
}

//directory things
struct CateSysDirectory {
    DIR* d;
};

struct CateSysDirectory* cate_sys_open_dir(const char* path) {
    DIR* d = opendir(path);
    if(!d) return 0;

    struct CateSysDirectory* result = malloc(sizeof(*result));
    result->d = d;
    return result;
}

void cate_sys_dir_close(struct CateSysDirectory* dir) {
    closedir(dir->d);
    free(dir);
}

char cate_sys_dir_get(struct CateSysDirectory* dir,
                            struct CateSysDirEntry* ent) {
restart:;
    struct dirent *dent = readdir(dir->d);
    
    if(!dent) {
        ent->type = CATE_SYS_DIRENT_NONE;
        return 0;
    }
    
    if(strncmp(dent->d_name, ".", 2) == 0
    || strncmp(dent->d_name, "..", 3) == 0)
        goto restart;

    ent->name = dent->d_name;
    ent->type = (dent->d_type == DT_DIR)
            ? CATE_SYS_DIRENT_DIR
            : CATE_SYS_DIRENT_FILE;
    return 1;
}

int cate_sys_remove(const char* path) {
    if(cmd_args.flags & CMD_DRY_RUN) {
        printf("rm %s\n", path);
        return 1;
    }
    if(strncmp(path, "/", 2) == 0 || strncmp(path, "/home", 6) == 0) {
        cate_error("script tried to remove system directories.");
    }

    //TODO: implement remove()
    return 1;
}

//processes stuff
static __pid_t _fork(const char* for_what) {
    __pid_t pid = fork();
    if(pid == -1) {
        cate_error("failed to create new process for %s", for_what);
    }
    return pid;
}

static inline int get_exit_code(int status) {
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return 1;
}

struct CateSysProcess cate_sys_process_create(char* const* args) {
    struct CateSysProcess proc = {0};
    proc.id = _fork("running a command");
    if(proc.id == 0) {
        execvp(args[0], &args[0]);
        cate_error("program \"%s\" is not installed", args[0]);
    }
    return proc;
}

int cate_sys_has_process_exited(struct CateSysProcess* p) {
    if(p->id == 0) return 1;
    pid_t ret = waitpid(p->id, &p->status, WNOHANG);
    assert(ret != -1 && "you forgot to clear the process struct");

    if(ret == p->id) {
        p->exit_code = WEXITSTATUS(p->status);
        return 1;
    }
    return 0;
}

int cate_sys_process_wait(struct CateSysProcess* p) {
    pid_t ret = waitpid(p->id, &p->status, 0);
    return get_exit_code(p->status);
}

void cate_sys_convert_path(char* posix, char* new) {
    //POSIX paths stay the same.
    return;
}