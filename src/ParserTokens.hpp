#ifndef PARSER_TOKENS
#define PARSER_TOKENS
#include "inc.hpp"

/*
	why hello there!
	i know most of this is bad, i honestly don't care too much because it's my first project.
*/

static string token_names[] = {
	"end of file",
	"a '.'",
	"a ','",
	"a ';'",
	"a '='",
	"a '{'",
	"a '}'",
	"a '('",
	"a ')'",
	"\"Project\"",
	"\"Library\"",
	"\"static\"",
	"\"dynamic\"",
	"\"recursive\"",
	"a string literal",
	"an identifier",
};
	
struct ParserToken
{
	enum ParserTokens
	{
		END,
		DOT,
		COMMA,
		SEMICOLON,
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
	} type;
	std::string value;
	int in_line;
};

#endif //PARSER_TOKENS
