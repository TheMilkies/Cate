#include "Parser/Parser.hpp"

using namespace Util;

void Parser::expect(ParserTokenKind type)
{
	current = next(); 

	if (current.type != type)
	{
		error(current.line, "Expected " + token_names[type] +
					" but got " + token_names[current.type]);
	}
}

void Parser::expect(ParserTokenKind type, ParserTokenKind type2)
{
	current = next();

	if (current.type != type && current.type != type2)
	{
		error(current.line, "Expected " + token_names[type] + " or " + token_names[type2] +
					" but got " + token_names[current.type]);
	}
}

bool Parser::expect_bool()
{
	current = next();

	if (current.type != B_TRUE && current.type != B_FALSE)
		fatal("Expected a boolean (true | false) value for " PURPLE + child + COLOR_RESET);

	return (current.type == B_TRUE);
}

bool Parser::expect_type()
{
	current = next();

	if (current.type != STATIC && current.type != DYNAMIC)
		error(current.line, "Expected a LibraryType (static | dynamic) value");

	return current.type == STATIC;
}

void Parser::expect_and_then(ParserTokenKind type, ParserTokenKind type2)
{
	current = next();

	if (current.type != type && peek().type != type2)
		error(current.line, "Expected " + token_names[type] + " and then " + token_names[type2]);
	
	current = next();
}

void Parser::expect(ParserTokenKind type, ParserTokenKind type2, ParserTokenKind type3)
{
	current = next();

	if (current.type == END)
		fatal("Unexpected end of file");

	if (current.type != type && current.type != type2 && current.type != type3)
	{
		fatal("Expected " + token_names[type] + " or " + token_names[type2] + " or " + token_names[type3] +
					" but got " + token_names[current.type]);
	}
}

void Parser::expect_string_array()
{
	current = next();

	if (current.type == END)
		fatal("Unexpected end of file");

	if (current.type != STRING_LITERAL && current.type != COMMA && current.type != RCURLY)
		fatal("Expected a string array ( `{\"like\", \"this\"}` )");
}

void Parser::expect_string_recursive_array()
{
	current = next();

	if (current.type == END)
		fatal_error(tokens[index-1].line, "Unexpected end of file");

	if (current.type != STRING_LITERAL && current.type != RECURSIVE && current.type != COMMA && current.type != RCURLY)
	{
		fatal("Expected a string array ( `{\"like\", \"this\"}` ) or "
						 hl_func("recursive()"));
	}
}

void Parser::expect(ParserTokenKind type, ParserTokenKind type2, ParserTokenKind type3, ParserTokenKind type4)
{
	if (tokens[index-1].type == END)
		fatal_error(current.line, "Unexpected end of file");

	current = next();

	if (current.type != type && current.type != type2 && current.type != type3 && current.type != type4)
	{
		error(current.line, "Expected " + token_names[type] + " or " + token_names[type2] + " or " + token_names[type3] +
					" or " + token_names[type4] + " but got " + token_names[current.type]);
	}
}

void Parser::optional_rparen()
{
	if (current.type != RPAREN)
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
	current = next();
	if(current.type != LPAREN || peek().type != STRING_LITERAL)
	{
		fatal("Expected a string inside parenthesis " hl_func("like(\"this\")"));
	}

	current = next();
	ParserToken to_return = current;

	current = next();
	optional_rparen();

	return to_return;
}