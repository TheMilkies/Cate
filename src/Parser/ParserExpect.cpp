#include "Parser/Parser.hpp"

using namespace Util;

void Parser::expect(ParserTokenKind type)
{
	next(); 

	if (!match(type))
	{
		error(current.line, "Expected " + token_names[type] +
					" but got " + token_names[current.type]);
	}
}

void Parser::expect(ParserTokenKind type, ParserTokenKind type2)
{
	next();

	if (!match(type) && !match(type2))
	{
		error(current.line, "Expected " + token_names[type] + " or " + token_names[type2] +
					" but got " + token_names[current.type]);
	}
}

bool Parser::expect_bool()
{
	next();

	if (!match(B_TRUE) && !match(B_FALSE))
		fatal("Expected a boolean (true | false) value for " PURPLE + child + COLOR_RESET);

	return match(B_TRUE);
}

bool Parser::expect_type()
{
	next();

	if (!match(STATIC) && !match(DYNAMIC))
		error(current.line, "Expected a LibraryType (static | dynamic) value");

	return match(STATIC);
}

void Parser::expect_and_then(ParserTokenKind type, ParserTokenKind type2)
{
	next();

	if (!match(type) && peek().type != type2)
		error(current.line, "Expected " + token_names[type] + " and then " + token_names[type2]);
	
	next();
}

void Parser::expect(ParserTokenKind type, ParserTokenKind type2, ParserTokenKind type3)
{
	next();

	if (match(END))
		fatal("Unexpected end of file");

	if (!match(type) && !match(type2) && !match(type3))
	{
		fatal("Expected " + token_names[type] + " or " + token_names[type2] + " or " + token_names[type3] +
					" but got " + token_names[current.type]);
	}
}

void Parser::expect_string_array()
{
	next();

	if (match(END))
		fatal("Unexpected end of file");

	if (!match(STRING_LITERAL) && !match(COMMA) && !match(RCURLY))
		fatal("Expected a string array ( `{\"like\", \"this\"}` )");
}

void Parser::expect_string_recursive_array()
{
	next();

	if (match(END))
		fatal_error(tokens[index-1].line, "Unexpected end of file");

	if (!match(STRING_LITERAL) && !match(RECURSIVE) && !match(COMMA) && !match(RCURLY))
	{
		fatal("Expected a string array ( `{\"like\", \"this\"}` ) or "
						 hl_func("recursive()"));
	}
}

void Parser::expect_library_recursive_array()
{
	next();

	if (match(END))
		fatal_error(tokens[index-1].line, "Unexpected end of file");

	if (!match(STRING_LITERAL) && !match(RECURSIVE) && !match(IDENTIFIER) && !match(COMMA) && !match(RCURLY))
	{
		fatal("Expected a string array ( `{\"like\", \"this\"}` ) or "
						 hl_func("recursive()") 
						 " or an identifier");
	}
}

void Parser::expect(ParserTokenKind type, ParserTokenKind type2, ParserTokenKind type3, ParserTokenKind type4)
{
	if (peek().type == END)
		fatal_error(current.line, "Unexpected end of file");

	next();

	if (!match(type) && !match(type2) && !match(type3) && !match(type4))
	{
		error(current.line, "Expected " + token_names[type] + " or " + token_names[type2] + " or " + token_names[type3] +
					" or " + token_names[type4] + " but got " + token_names[current.type]);
	}
}

void Parser::optional_rparen()
{
	if (!match(RPAREN))
	{
		current = tokens[index -= 1]; //go back one
		warn(current.line, "Missing ')'");
	}
}

void Parser::void_function()
{
	//it already checks if the next is '(', so we can just skip 2
	skip(2);
	optional_rparen();
}

//expects '(' string_literal ')' and then returns the string_literal token
ParserToken Parser::string_function()
{
	next();
	if(!match(LPAREN) || peek().type != STRING_LITERAL)
		fatal("Expected a string inside parenthesis " hl_func("like(\"this\")"));

	ParserToken to_return = tokens[++index];

	next();
	optional_rparen();

	return to_return;
}