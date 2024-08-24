#ifndef CATE_CLASS_H
#define CATE_CLASS_H
#include "string_table.h"
#include "target.h"
#include <stdint.h>

enum {
    CLASS_PROJECT = 0,
    CLASS_LIBRARY = 1,
};
typedef uint8_t ClassKind;

enum {
    LIBRARY_STATIC = 0,
    LIBRARY_DYNAMIC = 1,
};
typedef uint8_t LibraryKind;

enum {
    CLASS_BOOL_NONE = 0,
    CLASS_BOOL_HAS_ISSUE = 1 << 0,
    CLASS_BOOL_THREAD = 1 << 1,
    CLASS_BOOL_SMOL = 1 << 2,
    CLASS_BOOL_LINK = 1 << 3,
    CLASS_BOOL_BUILT = 1 << 4,
    CLASS_BOOL_AUTO = 1 << 5,
    CLASS_IBOOL_RELINK = 1 << 6,
    CLASS_IBOOL_TYPE_CHANGED = 1 << 7,
    CLASS_BOOL_END,

    CLASS_BOOLS_DEFAULT =
        CLASS_BOOL_LINK | CLASS_BOOL_AUTO,
};
typedef uint16_t ClassBools;

typedef struct {
    CStringArray command_template;

    SavedStringIndexes includes, defines, files, libraries, objects;

    string_view name, out_name, compiler, build_dir, standard,
                flags, final_flags;
    
    //maybe this should be an SSI?
    CStringArray loaded_lib_paths, all_libraries;

    struct {
        union {
            LibraryKind kind;
        } lib;
    } as;
    
    ClassBools bools;
    ClassKind kind;
} CateClass;

void class_build(CateClass* c);
void class_clean(CateClass* c);
void class_install(CateClass* c);
void class_change_type(CateClass* c, LibraryKind type);
void class_change_libraries(CateClass* c);

struct FileBuilder;
typedef struct {
    da_type(CateClass) classes;
    da_type(struct CateFullPath) loaded_files;
    StringTable st;
    struct FileBuilder* builders;
} CateContext;
//The context is global so we could reuse the data everywhere.
extern CateContext ctx;

void context_free();
void context_reset();

void save_string(string_view* s, SavedStringIndexes* arr);

#endif // CATE_CLASS_H