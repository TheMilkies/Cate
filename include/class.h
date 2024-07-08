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
    CLASS_BOOL_END,

    CLASS_BOOLS_DEFAULT =
        CLASS_BOOL_LINK | CLASS_BOOL_AUTO,
};
typedef uint8_t ClassBools;

typedef struct {
    CStringArray command_template;

    SavedStringIndexes includes, defines, files, libraries, objects;

    string_view name, out_name, compiler, build_dir, standard,
                flags, final_flags;

    struct {
        union {
            LibraryKind kind;
        } lib;
    } as;
    
    ClassBools bools;
    ClassKind kind;
} CateClass;

struct CateFullPath {
    char x[PATH_MAX];
};

typedef struct {
    da_type(CateClass) classes;
    da_type(struct CateFullPath) loaded_files;
    StringTable st;
} CateContext;
//The context is global so we could reuse the data everywhere.
extern CateContext ctx;

void context_free();
void context_reset();

#endif // CATE_CLASS_H