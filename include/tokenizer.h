#ifndef CATE_TOKENIZER_H
#define CATE_TOKENIZER_H
#include "vendor/string_view.h"
#include "vendor/dynamic_array.h"
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
    string_view text;
	size_t line;
    TokenKind kind;
} Token;
typedef da_type(Token) TokensArray;

const char* tok_as_text(TokenKind k);
uint8_t cate_tokenize(string_view *line, TokensArray *tokens);

#endif // CATE_TOKENIZER_H