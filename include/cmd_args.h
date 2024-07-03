#ifndef CATE_CMD_ARGS_H
#define CATE_CMD_ARGS_H
#include <stdint.h>

enum {
    CMD_NONE = 0,
    CMD_DRY_RUN = 1 << 0,
    CMD_ALWAYS_INSTALL = 1 << 1,
    CMD_NEVER_INSTALL = 1 << 2,
    CMD_DISABLE_SYSTEM = 1 << 3,
    CMD_FORCE_SMOLIZE = 1 << 4,
    CMD_FORCE_REBUILD = 1 << 5,
};

typedef struct CmdArgs {
    int32_t thread_count;
    uint8_t flags;
} CmdArgs;

//Yes it's a global variable, bite me!
extern CmdArgs cmd_args;

#endif // CATE_CMD_ARGS_H