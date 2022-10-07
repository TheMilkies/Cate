#include "Parser.hpp"
#include "Lexer.hpp"

#include "Project.hpp"
#include "Library.hpp"

/*
	why hello there!
	i know most of this is bad, i honestly don't care too much because it's my first project. -yogurt
*/

/*
	string_function: '(' string_literal ')' {return string_literal;}
	void_function: '(' ')'
*/

Parser::Parser(const string& file_name)
{
	std::ifstream file(file_name);
	if (file.fail())
		Util::command_error("Cannot open file \"" + file_name + "\"");
	
	yyFlexLexer* lexer = new yyFlexLexer(file, std::cout); //create the lexer
	tokens.reserve(128); //optimization
	classes.reserve(8);

	ParserToken temp;
	
	while (temp.type = (ParserTokenKind)lexer->yylex()) //Flex doesn't work in a way you might expect, so we make it easier to work with
	{
		temp.value = ""; //reset to save some memory
		temp.line = lexer_line; //defined in lexer.l
		
		if (temp.type == STRING_LITERAL)
		{
			//string literals have the quotes (example: "string") so we need to remove them
			temp.value = lexer->YYText()+1;
			temp.value.pop_back();
		}
		else if (temp.type == IDENTIFIER)
		{
			//identifiers are just kept as themselves
			temp.value = lexer->YYText();
		}

		tokens.push_back(temp); //add the token
	}
	
	delete lexer; //delete the monstrosity and free its memory

	parse(); //start parsing
}

__attribute__((optimize("unroll-loops")))
Parser::~Parser()
{
	for(auto &c : classes)
		delete c.second; //free the pointers
}

void Parser::define(const string &identifier)
{
	if (is_defined(identifier))
		Util::fatal_error(current.line, "\"" + identifier + "\" was already defined");
	
	//this is technically a factory... oh well
	if (temp_type == PROJECT)
		classes[identifier] = new Project;
	else //library
		classes[identifier] = new Library;

	current_class = classes[current.value]; //set the pointer to the current class

	current_class->name = identifier; //set its name to the identifier
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

void Parser::declare()
{
	//declaration: type identifier {define(type, identifier);}
	temp_type = current.type; //library or project
	expect(IDENTIFIER);
	define(current.value);
}

void Parser::declare_library()
{
	/* library_declaration: library identifier '(' lib_type ')' {
			define(type, identifier);
			current.type = lib_type;
		} */

	declare(); //already wrote that code, reusing it.

	expect(LPAREN);
	expect_type();

	//temp_type = current.type;
	current_class->is_static = (current.type == STATIC); //if current token is static, set the type to static.

	expect(RPAREN);
}

void Parser::parse()
{
	current = next(); //gets the first token (0)
	
	string parent; parent.reserve(16); child.reserve(16); //parent.child
	while (current.type != END) //END = end of file
	{
		switch (current.type)
		{
		case PROJECT:
			declare();
			break;

		case LIBRARY:
			declare_library();
			break;

		case IDENTIFIER:
			/*property: string_literal '.' string_literal*/
			parent = current.value;

			if (!is_defined(parent))
				Util::fatal_error(current.line, "\"" + parent + "\" is not defined.");

			if (current_class->name != parent)
				current_class = classes[parent];

			expect_and_then(DOT, IDENTIFIER);
			child = current.value;

			//object_method: property function_parens
			//this is a dumb and smart optimisation
			if (tokens[index+1].type == LPAREN) object_method();
			else
			{
				//assignment: property '=' expr
				expect(ASSIGN);

				//expr: string_literal | recursive string_function | '{' expr '} | lib_type'
				if(special_case()); // handled there
				else
				{
					expect(STRING_LITERAL, LCURLY, RECURSIVE);

					if (current.type == STRING_LITERAL)
					{
						current_class->set_property(current.line, child, current.value); //set current property to the string literal
					}
					else 
					{
						current_class->clear_property(current.line, child); //clear array
						if (current.type == LCURLY)
							array(); //start the array
						else if (current.type == RECURSIVE)
							recursive();
					}
				}
			}
			
			break;
		
		case SYSTEM:
			if (system_allowed)
			{
				Util::user_system(current.line, string_function().value);
			}
			else
			{
				current = tokens[index += 3];
			}
			break;

		case SEMICOLON: //ignored
			break;

		default:
			Util::error(current.line, "Did not expect " + token_names[current.type] + ".");
			break;
		}

		current = next(); // get next token
	}

	if (parser_exit) exit(1); //if there was a non-fatal error, exit. 
}

void Parser::array()
{
	vector<string>& current_property = current_class->get_array_property(current.line, child);
	//this is an expr, continuing '{' expr '} but doesn't allow nested arrays.
	while (current.type != RCURLY)
	{
		expect(STRING_LITERAL, RECURSIVE, IDENTIFIER, COMMA, RCURLY);
		if (current.type == RECURSIVE)
		{
			recursive();
		}
		else if (current.type == STRING_LITERAL)
		{
			current_property.emplace_back(current.value);
		}
		else if (current.type == IDENTIFIER)
		{
			if (child == "libs")
			{
				if (is_defined(current.value))
					current_property.emplace_back(
							classes[current.value]->out_name);
				else
					Util::fatal_error(current.line, "\"" + current.value + "\" is not defined");
			}
			else
			{
				Util::fatal_error(current.line, "classes can only be included in the " highlight_var("libraries")
													" (or " highlight_var("libs") ") property.");
			}
		}
	}
}

bool Parser::special_case()
{
	if (child == "type")
	{
		expect_type();
		current_class->is_static = (current.type == STATIC);
		current_class->needs_rebuild += (!Util::file_exists(current_class->out_name.c_str())); 
		return true;
	}
	else if (child == "threading")
	{
		expect_bool();
		current_class->threading = (current.type == TRUE);
		return true;
	}
	else if (child == "smolize")
	{
		expect_bool();
		current_class->size_optimize = (current.type == TRUE);
		return true;
	}
	else if (child == "link")
	{
		expect_bool();
		current_class->link = (current.type == TRUE);
		return true;
	}

	return false;
}