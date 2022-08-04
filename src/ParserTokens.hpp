#ifndef PARSER_TOKENS
#define PARSER_TOKENS
#include "inc.hpp"

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
	"\"string\"",
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
		STRING,
		SYSTEM,
	} type;
	std::string value;
	int in_line;
};

#endif //PARSER_TOKENS
