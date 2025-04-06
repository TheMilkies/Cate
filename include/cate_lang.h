#ifndef CATE_LANG_H
#define CATE_LANG_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "libcate_sys.h"

/*
	Almost an entire Cate in a module!
	This module contains: catel, tokenizer, parser, and virtual machine spec.

	You might be wondering why Cate needs a virtual machine, and that's fair.
	- It's more efficient than direct interpretation. 
	- It allows for a lot more flexibility. We might make cate2sh and cate2ps.
*/

/*----------.
| tokenizer |
`---------*/
enum {
    CTOK_NONE = 0,
	CTOK_DOT,
	CTOK_ASSIGN,
	CTOK_LCURLY,
	CTOK_RCURLY,
	CTOK_LPAREN,
	CTOK_RPAREN,
	CTOK_PROJECT,
	CTOK_LIBRARY,
	CTOK_STATIC,
	CTOK_DYNAMIC,
	CTOK_STRING_LITERAL,
	CTOK_IDENTIFIER,
	CTOK_TRUE,
	CTOK_FALSE,
	CTOK_IF,
	CTOK_ELSE,
	CTOK_EXCLAMATION_MARK,
    
    CTOK_COUNT_SIZE,
};

typedef uint8_t TokenKind;
typedef uint32_t TokenID;
typedef struct {
    //16 million lines should be more than enough for cate.
    uint32_t line : 24,
             kind : 8;
} Token;
typedef da_type(Token) TokensArray;

/*
	Only two kinds (identifiers and strings) have a value, so the rest will
	have a null field, this is not ideal because we'd be wasting cache lines.
	
	We have a separate array instead!
	We need to keep track of the current token and the current value, which is
	just incrementing two integers (see parser).
	Getting the value is a bit more annoying but we can fit 16 tokens in a
	cache line and we don't waste (1.5 * token_count * 16) bytes.
*/
typedef da_type(cate_sv) TokenValuesArray;

void cate_tokenize(cate_sv *line, TokensArray *tokens,
    TokenValuesArray* values);
const char* ctok_as_text(TokenKind k);

/*------.
| catel |
`-----*/
typedef struct {
    CateSysPath dir, def;
} Catel;

/// @brief Parse catel from file
void catel_init(Catel* catel);

/*--------.
| context |
`-------*/
typedef struct {
    da_type(uint32_t) classes;
    da_type(CateSysPath) opened_files;
	Catel* catel;
} CateContext;

void cate_context_destroy(CateContext* context);

/*--------.
| cate IR |
`-------*/
enum {
    CCLASS_PROJECT = 0,
    CCLASS_LIB_STATIC,
    CCLASS_LIB_DYNAMIC,
    CCLASS__END,
};

enum {
	CPROP_COMPILER = 0,
	CPROP_BUILD_DIR,
	CPROP_STD,
	CPROP_LINKER,
	CPROP_LINKER_SCRIPT,
	CPROP_OUT_NAME,
	CPROP_FILES,
	CPROP_FLAGS,
	CPROP_LINKER_FLAGS,
	CPROP_LIBRARIES,
	CPROP_INCLUDES,
};

enum {
	CBOOL_AUTO = 0,
	CBOOL_LINK,
	CBOOL_THREAD,
	CBOOL_SMOL,
};

typedef struct {
	uint32_t op;
} CInst_S;

enum {
	CINST_NOP = 0,
	CINST_INIT,

	CINST_NEW_CLASS,
	CINST_SET_PROPERTY,
	CINST_SET_GLOBAL_PROPERTY,
	CINST_SET_BOOL,
	CINST_SET_KIND,

	CINST_JUMP,
	CINST_JUMP_T,
	CINST_JUMP_F,
};

#endif // CATE_LANG_H