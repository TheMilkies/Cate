#ifndef LIB_CATE_H
#define LIB_CATE_H
#include <vendor/dynamic_array.h>
#include <vendor/string_view.h>
#include <stdint.h>

/*--------.
| strings |
`-------*/
typedef da_type(char*) StringsArray;
char* cate_string_clone(char* s);
char* cate_string_build(int count, ...);
char* sv_clone_as_cstr(string_view* v);

enum {
    C_CLASS_PROJECT = 0,
    C_CLASS_LIB_STATIC,
    C_CLASS_LIB_DYNAMIC,
};
typedef uint8_t CateClassKind;

enum {
    CATE_FLAG_AUTO = 1 << 0,
    CATE_FLAG_SMOL = 1 << 1,
    CATE_FLAG_LINK = 1 << 2,
    CATE_FLAGS_DEFAULT =
        CATE_FLAG_AUTO | CATE_FLAG_LINK,
};
typedef uint8_t CateFlags;

typedef struct {
    char *compiler, *build_dir, *std;

    uint32_t thread_count;
    CateFlags options;
} CateGlobals;
extern CateGlobals* c_current_globals;

/*--------.
| globals |
`-------*/
void cate_globals_init(CateGlobals* g);
void cate_globals_free(CateGlobals* g);

/*-------.
| system |
`------*/
int cs_create_directory(char* dir);
int cs_file_exists(char* file);
int cs_newer_than(char* file1, char* file2);
int cs_copy(char* file1, char* file2);
int cs_move(char* file1, char* file2);
int cs_remove(char* file1);

#endif // LIB_CATE_H