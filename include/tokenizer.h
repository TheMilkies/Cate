#ifndef CATE_TOKENIZER_H
#define CATE_TOKENIZER_H
#include <vendor/string_view.h>
#include <vendor/dynamic_array.h>
#include "cate_error.h"
#include "common.h"

#include <stdint.h>

enum {
    TOK_NONE = 0,
	TOK_DOT,
	TOK_ASSIGN,
	TOK_LCURLY,
	TOK_RCURLY,
	TOK_LPAREN,
	TOK_RPAREN,
	TOK_PROJECT,
	TOK_LIBRARY,
	TOK_STATIC,
	TOK_DYNAMIC,
	TOK_RECURSIVE,
	TOK_STRING_LITERAL,
	TOK_IDENTIFIER,
	TOK_TRUE,
	TOK_FALSE,
	TOK_IF,
	TOK_ELSE,
	TOK_EXCLAMATION_MARK,
    
    TOK_COUNT_SIZE,
};
typedef uint8_t TokenKind;

typedef struct {
    //16 million lines should be more than enough.
    uint32_t line : 24,
             kind : 8;
} Token;
typedef da_type(Token) TokensArray;
typedef da_type(string_view) TokenValuesArray;

void cate_tokenize(string_view *line, TokensArray *tokens,
    TokenValuesArray* values);

#endif // CATE_TOKENIZER_H