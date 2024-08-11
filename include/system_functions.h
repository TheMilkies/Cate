#ifndef CATE_SYSTEM_FUNCTIONS_H
#define CATE_SYSTEM_FUNCTIONS_H
#include <stdlib.h>

#ifndef _WIN32
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 1
#endif
#include <limits.h>
#endif

#ifndef PATH_MAX
#define PATH_MAX 2048
#endif

/*
    This is an abstraction layer because POSIX and Windows can't agree
    on anything. It should make things easier though.
    
    --sonu
*/

/*
    what started off as a build system that'd be simpler than existing ones
    has turned into an operating system emulator at this point.

    oh how naive i was back then, i was so happy with my little C++
    build system. now i have to support windows and therefore make an
    abstraction layer.

    --yogurt
*/

struct CateFullPath {
    char x[PATH_MAX];
};

int cate_sys_mkdir(char* path);
int cate_sys_copy(char* path1, char* path2);
int cate_sys_move(char* path1, char* path2);
int cate_sys_smolize(char* path);
int cate_sys_system(const char* cmd);
int cate_sys_remove(const char* path);
int cate_sys_file_exists(const char* path);
size_t cate_get_modified_time(const char* path);
int cate_is_file_newer(const char* path1, const char* path2);
size_t cate_sys_get_core_count();

enum {
    CATE_SYS_DIRENT_NONE = 0,
    CATE_SYS_DIRENT_FILE,
    CATE_SYS_DIRENT_DIR,
};
struct CateSysDirEntry {
    char* name;
    char type;
};

struct CateSysDirectory;

/// @brief Open a directory from path
/// @param path The path
/// @return NULL if error, else an opaque object.
struct CateSysDirectory* cate_sys_open_dir(const char* path);
/// @brief Closes a directory
/// @param dir The directory
void cate_sys_dir_close(struct CateSysDirectory* dir);
/// @brief Get an entry from a directory
/// @param dir The directory
/// @param ent The entry to store it in
/// @return 1 if there are more files, 0 if reached the end of listing.
char cate_sys_dir_get(struct CateSysDirectory* dir,
                        struct CateSysDirEntry* ent);

void cate_sys_convert_path(char* posix, struct CateFullPath* new);

typedef size_t CateSysProcessID;
struct CateSysProcess {
    CateSysProcessID id;
    union {
        int status;
        void* handle;
    };
    int exit_code;
};

/// @brief Create a new process and get its
/// @param args The arg at [0] is the program's name, the rest are the args.
/// @return The new process.
struct CateSysProcess cate_sys_process_create(char* const* args);
/// @brief Check if the process has exited and store its exit code.
/// @param p The process
/// @return 1 if true, 0 if false
int cate_sys_has_process_exited(struct CateSysProcess* p);
/// @brief Wait until the given process exits
/// @param p The process
/// @return exit code of the process
int cate_sys_process_wait(struct CateSysProcess* p);

#endif // CATE_SYSTEM_FUNCTIONS_H