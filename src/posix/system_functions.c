#include "system_functions.h"
#include <sys/sysinfo.h> //get_nprocs()
#include <unistd.h>

size_t cate_sys_get_core_count() {
    return get_nprocs();
}

int cate_sys_file_exists(const char* path) {
    return access(path, F_OK) == 0;
}