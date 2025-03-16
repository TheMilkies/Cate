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

/*------.
| paths |
`-----*/
typedef struct {
    char x[FILENAME_MAX];
    size_t length;
} CateSysPath;
// void cs_path_translate(CateSysPath* p, char* text);
void cs_path_append(CateSysPath* p, char* text);
void cs_path_directory_separator(CateSysPath* p);

enum {
    C_CLASS_PROJECT = 0,
    C_CLASS_LIB_STATIC,
    C_CLASS_LIB_DYNAMIC,
    C_CLASS__END,
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

    CateFlags options;
} CateGlobals;
extern CateGlobals* c_current_globals;

void c_globals_init(CateGlobals* g);
void c_globals_free(CateGlobals* g);

extern uint8_t c_cmd_flags;
enum {
    C_CMD_DRY_RUN = 1 << 0,
    C_CMD_ALWAYS_INSTALL = 1 << 1,
    C_CMD_NEVER_INSTALL = 1 << 2,
};
/*--------.
| classes |
`-------*/
typedef struct {
    StringsArray files, object_files, flags,
    link_flags, libraries, located_libraries, library_paths, includes;

    char *name, *out_name, *compiler, *build_dir, *std,
         *linker, *linker_script;

    CateFlags options;
    CateClassKind kind;
} CateClass;

CateClass c_class(char* name, CateClassKind kind);
void c_class_build(CateClass* c);
void c_class_clean(CateClass* c);
void c_class_install(CateClass* c);
void c_add_file(CateClass* c, char* file);
void c_add_include(CateClass* c, char* path);
void c_add_library(CateClass* c, char* name, CateClassKind k);
void c_change_library_kind(CateClass* c, CateClassKind k);
void c_set_standard(CateClass* c, char* std);
void c_set_flags(CateClass* c, char* flags);
void c_add_flag(CateClass* c, char* flag);
void c_set_link_flags(CateClass* c, char* flags);
void c_add_link_flag(CateClass* c, char* flag);
void c_class_free(CateClass* c);

/*-------.
| system |
`------*/
int cs_is_admin();
int cs_create_directory(char* dir);
int cs_file_exists(char* file);
int cs_newer_than(char* file1, char* file2);
int cs_copy(char* file1, char* file2);
int cs_move(char* file1, char* file2);
int cs_remove(char* file);
int cs_remove_single(const char* file);
int cs_smolize(char* file);

#endif // LIB_CATE_H
