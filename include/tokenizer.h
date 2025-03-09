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

typedef struct {
    //16 million lines should be more than enough for cate.
    uint32_t line : 24,
             kind : 8;
} Token;
typedef da_type(Token) TokensArray;

/*
	this might seem weird, but it's actually the least wasteful way to store
	token values. it might be even faster than using a normal array!
	roughly half of a catefile's tokens don't have a value, this also avoids
	the cache misses of array lookup.
*/
typedef uint32_t TokenID;
typedef struct {
	string_view text;
	TokenID id;
} TokenValue;
typedef da_type(TokenValue) TokenValuesArray;

string_view get_value_from_id(TokenValuesArray* values, TokenID id,
							  TokenID last);

void cate_tokenize(string_view *line, TokensArray *tokens,
    TokenValuesArray* values);
const char* tok_as_text(TokenKind k);

#endif // CATE_TOKENIZER_H
