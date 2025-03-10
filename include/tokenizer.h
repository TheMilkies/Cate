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
const char* tok_as_text(TokenKind k);

#endif // CATE_TOKENIZER_H
