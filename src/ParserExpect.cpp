#include "Parser.hpp"

void Parser::expect(ParserTokenKind type)
{
	current = next(); //get next token to compare

	if (current.type != type)
	{
		Util::error(current.line, "Expected " + token_names[type] +
					" but got " + token_names[current.type]);
	}
}

void Parser::expect(ParserTokenKind type, ParserTokenKind type2)
{
	current = next();

	if (current.type != type && current.type != type2)
	{
		Util::error(current.line, "Expected " + token_names[type] + " or " + token_names[type2] +
					" but got " + token_names[current.type]);
	}
}

bool Parser::expect_bool()
{
	current = next();

	if (current.type != B_TRUE && current.type != B_FALSE)
	{
		Util::fatal_error(current.line, "Expected a boolean (true | false) value");
	}
	return (current.type == B_TRUE);
}

void Parser::expect_type()
{
	current = next();

	if (current.type != STATIC && current.type != DYNAMIC)
	{
		Util::error(current.line, "Expected a LibraryType (static | dynamic) value");
	}
}

void Parser::expect_and_then(ParserTokenKind type, ParserTokenKind type2)
{
	current = next();

	if (current.type != type && tokens[index+1].type != type2)
	{
		Util::error(current.line, "Expected " + token_names[type] + " and then " + token_names[type2]);
	}
	
	current = next();
}

void Parser::expect(ParserTokenKind type, ParserTokenKind type2, ParserTokenKind type3)
{
	current = next();

	if (current.type != type && current.type != type2 && current.type != type3)
	{
		Util::fatal_error(current.line, "Expected " + token_names[type] + " or " + token_names[type2] + " or " + token_names[type3] +
					" but got " + token_names[current.type]);
	}
}

void Parser::expect(ParserTokenKind type, ParserTokenKind type2, ParserTokenKind type3, ParserTokenKind type4)
{
	current = next();

	if (current.type != type && current.type != type2 && current.type != type3 && current.type != type4)
	{
		Util::error(current.line, "Expected " + token_names[type] + " or " + token_names[type2] + " or " + token_names[type3] +
					" or " + token_names[type4] + " but got " + token_names[current.type]);
	}
}

void Parser::expect(ParserTokenKind type, ParserTokenKind type2, ParserTokenKind type3, ParserTokenKind type4, ParserTokenKind type5)
{
	current = next();

	if (current.type != type && current.type != type2 && current.type != type3 && current.type != type4 && current.type != type5)
	{
		Util::error(current.line, "Expected " + token_names[type] + " or " + token_names[type2] + " or " + token_names[type3] +
					" or " + token_names[type4] + " or " + token_names[type5] + " but got " + token_names[current.type]);
	}
}

void Parser::void_function()
{
	//it already checks if the next is '(', so we can just skip 2
	current = tokens[index += 2];

	if (current.type != RPAREN)
	{
		Util::error(current.line,
					"Missing ')'");
	}
}

//expects '(' string_literal ')' and then returns the string_literal token
ParserToken Parser::string_function()
{
	expect_and_then(LPAREN, STRING_LITERAL);

	ParserToken to_return = current;
	if (tokens[index+1].type != RPAREN)
	{
		Util::error(current.line,
					"Missing ')'");
	}

	current = next();
	return to_return;
}