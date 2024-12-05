#ifndef LIB_CATE_H
#define LIB_CATE_H
#include <vendor/dynamic_array.h>
#include <vendor/string_view.h>
#include <stdint.h>

/*--------.
| strings |
`-------*/
typedef da_type(char*) StringsArray;
char* c_string_clone(char* s);
char* c_string_build(int count, ...);
char* sv_clone_as_cstr(string_view* v);

enum {
    C_CLASS_PROJECT = 0,
    C_CLASS_LIB_STATIC,
    C_CLASS_LIB_DYNAMIC,
};
typedef uint8_t CateClassKind;

enum {
    C_FLAG_AUTO = 1 << 0,
    C_FLAG_SMOL = 1 << 1,
    C_FLAG_LINK = 1 << 2,
    C_FLAG_BUILT = 1 << 3,
    C_FLAGS_DEFAULT =
        C_FLAG_AUTO | C_FLAG_LINK,
};
typedef uint8_t CateFlags;

/*--------.
| globals |
`-------*/
typedef struct {
    char *compiler, *build_dir, *std,
         *linker, *linker_script;

    uint32_t thread_count;
    CateFlags options;
} CateGlobals;
extern CateGlobals* c_current_globals;

void c_globals_init(CateGlobals* g);
void c_globals_free(CateGlobals* g);

/*--------.
| classes |
`-------*/
typedef struct {
    StringsArray files, object_files, flags,
    link_flags, libraries, library_paths, includes;

    char *name, *out_name, *compiler, *build_dir, *std,
         *linker, *linker_script;

    uint32_t thread_count;
    CateFlags options;
    CateClassKind kind;
} CateClass;

CateClass c_class(char* name, CateClassKind kind);
void c_class_build(CateClass* c);
void c_add_file(CateClass* c, char* file);
void c_class_free(CateClass* c);

/*-------.
| system |
`------*/
int cs_create_directory(char* dir);
int cs_file_exists(char* file);
int cs_newer_than(char* file1, char* file2);
int cs_copy(char* file1, char* file2);
int cs_move(char* file1, char* file2);
int cs_remove(char* file);
int cs_smolize(char* file);

#endif // LIB_CATE_H