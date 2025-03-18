#ifndef CATE_LANG_H
#define CATE_LANG_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <vendor/string_view.h>
#include <vendor/dynamic_array.h>

// almost an entire cate in one module!

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
typedef da_type(string_view) TokenValuesArray;

void cate_tokenize(string_view *line, TokensArray *tokens,
    TokenValuesArray* values);
const char* ctok_as_text(TokenKind k);

/*------.
| catel |
`-----*/
typedef struct {
    char x[FILENAME_MAX];
    size_t length;
} _CateSysPath;

typedef struct {
    _CateSysPath dir, def;
} Catel;

/// @brief Parse catel from file
void catel_init(Catel* catel);

/*--------.
| context |
`-------*/
typedef struct {
    da_type(uint32_t) classes;
    da_type(_CateSysPath) opened_files;
	Catel* catel;
} CateContext;

void cate_context_destroy(CateContext* context);

#endif // CATE_LANG_H