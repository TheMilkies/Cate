#pragma once
#include "Util.hpp"

#define allow_equ_for_enum(enum_name) inline enum_name operator|(enum_name lhs, enum_name rhs) {\
    return (enum_name)((uint8_t)lhs | (uint8_t)(rhs));\
}\
inline enum_name& operator|=(enum_name& lhs, enum_name rhs) {\
    lhs = lhs | rhs;\
    return lhs;\
}\
inline enum_name operator&(enum_name lhs, enum_name rhs) {\
    return (enum_name)((uint8_t)lhs & (uint8_t)(rhs));\
}\
inline enum_name& operator&=(enum_name& lhs, enum_name rhs) {\
    lhs = lhs & rhs;\
    return lhs;\
}

enum Flags: uint8_t
{
	no_flags = 0,
	system_blocked = 0b1,
	force_rebuild = 0b10,
	force_smol  = 0b100,
	dry_run  = 0b1000,
	always_allow_install  = 0b10000,
	always_deny_install  = 0b100000
};
allow_equ_for_enum(Flags)
#define flag(flag) (global_values.options & flag)

struct Global
{
	string compiler = "cc", standard, object_dir;
	bool smol = false, threading = false;

	Flags options = no_flags;
};

inline Global global_values;