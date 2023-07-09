#ifndef PARSER_TOKENS
#define PARSER_TOKENS
#include "colors.hpp"
#include <iostream>
using std::string;
typedef int32_t i32;
typedef uint8_t u8;

/*
	why hello there!
	i know most of this is bad, i honestly don't care too much because it's my first project. -yogurt
*/

static string token_names[] = {
	"end of file",
	"a '.'",
	"a ','",
	"a '='",
	"a '{'",
	"a '}'",
	"a '('",
	"a ')'",
	"\"Project\"",
	"\"Library\"",
	"\"static\"",
	"\"dynamic\"",
	hl_func("recursive"),
	"a string literal",
	"an identifier",
	hl_func("system"),
	"\"true\"",
	"\"false\"",
	hl_func("subcate"),
};

enum ParserTokenKind: u8
{
	END,
	DOT,
	COMMA,
	ASSIGN,
	LCURLY,
	RCURLY,
	LPAREN,
	RPAREN,
	PROJECT,
	LIBRARY,
	STATIC,
	DYNAMIC,
	RECURSIVE,
	STRING_LITERAL,
	IDENTIFIER,
	SYSTEM,
	B_TRUE,
	B_FALSE,
	SUBCATE,
};
	
struct ParserToken
{
	ParserTokenKind type;
	string text;
	i32 line;

	ParserToken(ParserTokenKind kind): type(kind){}
};

#endif //PARSER_TOKENS