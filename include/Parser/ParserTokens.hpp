#ifndef PARSER_TOKENS
#define PARSER_TOKENS
#include "inc.hpp"
#include "colors.hpp"

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

enum ParserTokenKind: uint8_t
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
	std::string value;
	int32_t line;
};

#endif //PARSER_TOKENS