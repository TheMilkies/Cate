#ifndef CATE_SYSTEM_FUNCTIONS_H
#define CATE_SYSTEM_FUNCTIONS_H
#include <stdlib.h>

/*
    This is an abstraction layer because POSIX and Windows can't agree
    on anything. It should make things easier though.
*/

size_t cate_sys_get_core_count();
int cate_sys_file_exists(const char* path);

#endif // CATE_SYSTEM_FUNCTIONS_H