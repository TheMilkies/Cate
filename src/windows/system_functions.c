#include "system_functions.h"
#include "cmd_args.h"
#include "error.h"
#include "target.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <tlhelp32.h>

static inline int get_exit_code(int status);

size_t cate_sys_get_core_count() {
    SYSTEM_INFO sysinfo = {0};
    return sysinfo.dwNumberOfProcessors*2;
}

int cate_sys_file_exists(const char* path) {
    cate_sys_convert_path(path);
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
    if (tmp[len - 1] == path_sep)
        tmp[len - 1] = 0;
    for (p = tmp + 1; *p; p++)
        if (*p == path_sep) {
            *p = 0;
            if(_mkdir(tmp) != 1) return 0;
            *p = path_sep;
        }
    return _mkdir(tmp) == 1;
}

int cate_sys_mkdir(char* path) {
    cate_sys_convert_path(path);
    if(cate_sys_file_exists(path)) return 1;
    if(cmd_args.flags & CMD_DRY_RUN) {
        printf("mkdir %s\n", path);
        return 1;
    }
    return _recursive_mkdir(path);
}

int cate_sys_copy(char* path1, char* path2) {
    cate_sys_convert_path(path1);
    cate_sys_convert_path(path2);
    if(cmd_args.flags & CMD_DRY_RUN) {
        printf("cp %s %s\n", path1, path2);
        return 1;
    }
    
    return CopyFile(path1, path2, FALSE);
}

int cate_sys_move(char* path1, char* path2) {
    if(cmd_args.flags & CMD_DRY_RUN) {
        printf("mv %s %s\n", path1, path2);
        return 1;
    }
    return MoveFile(path1, path2) == 0;
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
    HANDLE hFind;
    WIN32_FIND_DATA findData;
    int first_call; //because windows sucks
};

struct CateSysDirectory* cate_sys_open_dir(const char* path) {
    cate_sys_convert_path(path);
    struct CateSysDirectory* result = malloc(sizeof(*result));
    if (!result) return 0;

    char search_path[MAX_PATH] = {0};
    snprintf(search_path, sizeof(searchPath), "%s\\*", path);

    result->hFind = FindFirstFile(search_path, &result->findData);
    if (result->hFind == INVALID_HANDLE_VALUE) {
        free(result);
        return 0;
    }

    result->firstCall = 1;
    return result;
}

void cate_sys_dir_close(struct CateSysDirectory* dir) {
    FindClose(dir->hFind);
    free(dir);
}

char cate_sys_dir_get(struct CateSysDirectory* dir, struct CateSysDirEntry* ent) {
    while (1) {
        if (!dir->firstCall) {
            //get next file
            if (!FindNextFile(dir->hFind, &dir->findData)) {
                ent->type = CATE_SYS_DIRENT_NONE;
                return 0; // No more entries
            }
        } else {
            dir->firstCall = 0;
        }

        if (strcmp(dir->findData.cFileName, ".") == 0 ||
            strcmp(dir->findData.cFileName, "..") == 0) {
            continue;
        }

        // Fill the entry structure
        ent->name = dir->findData.cFileName;
        ent->type = (dir->findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    ? CATE_SYS_DIRENT_DIR
                    : CATE_SYS_DIRENT_FILE;
        return 1; // Successfully retrieved an entry
    }
}

int cate_sys_remove(const char* path) {
    if(cmd_args.flags & CMD_DRY_RUN) {
        printf("del %s\n", path);
        return 1;
    }
    
    if(strncmp(path, "/", 2) == 0 || strncmp(path, "/home", 6) == 0) {
        cate_error("script tried to remove system directories.");
    }

    //TODO: implement remove()
    return 1;
}

//processes stuff
struct CateSysProcess cate_sys_process_create(char* const* args) {
    struct CateSysProcess proc = {0};
    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);

    cate_sys_convert_path(path[0]);

    // Create the process
    if (!CreateProcess(
            args[0],
            NULL,
            NULL,
            NULL,
            FALSE,
            0,
            NULL,
            NULL,
            &si,
            &pi)
    ) {
        cate_error("program \"%s\" is not installed", args[0]);
        return proc;
    }

    proc.id = pi.dwProcessId;
    proc.handle = pi.hProcess;

    // Close the thread handle as we don't need it
    CloseHandle(pi.hThread);

    return proc;
}

int cate_sys_has_process_exited(struct CateSysProcess* p) {
    if (p->id == 0) return 1; // Process ID 0 indicates no process

    DWORD result = WaitForSingleObject(p->handle, 0);
    assert(result != WAIT_FAILED && "you forgot to clear the process struct");

    if (result == WAIT_OBJECT_0) {
        // Process has exited
        GetExitCodeProcess(p->handle, &p->exit_code);
        CloseHandle(p->handle);
        p->handle = NULL;
        return 1;
    }
    return 0;
}

void cate_sys_convert_path(char* path) {
    if(!path || !path[0]) return;
    size_t length = strlen(posix);
    if(path[0] == '/' || path[0] == '~')
        cate_error("path emulation does not support '%c']", path[0]);
    if(length >= 260)
        cate_error("path too long for windows");

    for (size_t i = 0; i < length; ++i)
        if(path[i] == '/')
            path[i] = '\\';
}

int cate_sys_process_wait(struct CateSysProcess* p) {
    DWORD result = WaitForSingleObject(p->handle, INFINITE);
    
    if (result == WAIT_OBJECT_0) {
        if (GetExitCodeProcess(p->handle, &p->exit_code)) {
            CloseHandle(p->handle);
            p->handle = 0;
            return p->exit_code;
        } else {
            return -1;
        }
    }
    
    //something went wrong
    return -1; 
}