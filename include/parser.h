#ifndef CATE_PARSER_H
#define CATE_PARSER_H
#include "tokenizer.h"
#include "libcate.h"
#include "catel.h"

/*
    The plan is to generate "generic cate bytecode".
    Generally, the tokenizer and parser should be in the same module.
    It's faster to compile to bytecode and run it than to directly interpret.
    This way we can also create tools like cate2sh and cate2ps.
*/

//CI = CateInst
//there are 3 kinds of instructions: N(ew), J(ump), L(oad)
enum {
    CI_PROP_COMPILER = 0,
    CI_PROP_OUT_NAME,
    CI_PROP_BUILD_DIR,
    CI_PROP_STD,
    CI_PROP_LINKER,
    CI_PROP_LINKER_SCRIPT,
    CI_PROP_FILES,
    CI_PROP_FLAGS,
    CI_PROP_LINKER_FLAGS,
    CI_PROP_LIBRARIES,
    CI_PROP_INCLUDES,
};

struct CI_New {
    uint32_t op : 4,
             kind : 2,
             reserved : 26;
};

struct CI_Jump {
    uint32_t op : 4,
             to : 28;
};

struct CI_Load {
    uint32_t op : 4,
             id : 28;
};

enum {
    CI_NOP = 0,
    CI_NEW,
    CI_LOAD_OBJECT,
    CI_SET_STRING,
    CI_APPEND_STRING,
    CI_CALL,
    CI_JUMP,
    CI_JUMP_T,
    CI_JUMP_F,
};


typedef struct {
    da_type(CateClass) classes;
    da_type(CateSysPath) opened_files;
} CateContext;

void cate_run(CateContext* ctx, string_view file_name);

#endif // CATE_PARSER_H