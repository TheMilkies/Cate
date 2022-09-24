#include "Parser.hpp"
#include "Lexer.hpp"

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
	
	while (temp.type = (ParserToken::ParserTokens)lexer->yylex()) //Flex doesn't work in a way you might expect, so we make it easier to work with
	{
		temp.value = ""; //reset to save some memory
		temp.in_line = lexer_line; //defined in lexer.l
		
		if (temp.type == ParserToken::STRING_LITERAL)
		{
			//string literals have the quotes (example: "string") so we need to remove them
			temp.value = lexer->YYText()+1;
			temp.value.pop_back();
		}
		else if (temp.type == ParserToken::IDENTIFIER)
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
		Util::fatal_error(current.in_line, "\"" + identifier + "\" was already defined");
	
	//this is technically a factory... oh well
	if (temp_type == ParserToken::PROJECT)
		classes[identifier] = new Project;
	else //library
		classes[identifier] = new Library;

	classes[identifier]->name = identifier; //set its name to the identifier
}

void Parser::void_function()
{
	//it already checks if the next is '(', so we can just skip 2
	current = tokens[index += 2];

	if (current.type != ParserToken::RPAREN)
	{
		Util::error(current.in_line,
					"Missing ')'");
	}
	
}

//expects '(' string_literal ')' and then returns the string_literal token
ParserToken Parser::string_function()
{
	expect(ParserToken::LPAREN);
	expect(ParserToken::STRING_LITERAL);
	ParserToken to_return = current;
	if (tokens[index+1].type != ParserToken::RPAREN)
	{
		Util::error(current.in_line,
					"Missing ')'");
	}
	current = next();
	return to_return;
}

void Parser::declare()
{
	//declaration: type identifier {define(type, identifier);}
	temp_type = current.type; //library or project
	expect(ParserToken::IDENTIFIER);
	define(current.value);
	current_class = classes[current.value]; //set the pointer to the current class
}

void Parser::declare_library()
{
	/* library_declaration: library identifier '(' lib_type ')' {
			define(type, identifier);
			current.type = lib_type;
		} */

	declare(); //already wrote that code, reusing it.

	expect(ParserToken::LPAREN);
	expect(ParserToken::STATIC, ParserToken::DYNAMIC);

	//temp_type = current.type;
	current_class->is_static = (current.type == ParserToken::STATIC); //if current token is static, set the type to static.

	expect(ParserToken::RPAREN);
}

void Parser::parse()
{
	current = next(); //gets the first token (0)
	
	string parent; parent.reserve(16); child.reserve(16); //parent.child
	while (current.type != ParserToken::END) //END = end of file
	{
		switch (current.type)
		{
		case ParserToken::PROJECT:
			declare();
			break;

		case ParserToken::LIBRARY:
			declare_library();
			break;

		case ParserToken::IDENTIFIER:
			/*property: string_literal '.' string_literal*/
			parent = current.value;

			if (!is_defined(parent))
				Util::fatal_error(current.in_line, "\"" + parent + "\" is not defined.");

			if (current_class->name != parent)
				current_class = classes[parent];

			expect(ParserToken::DOT);
			expect(ParserToken::IDENTIFIER);
			child = current.value;

			//object_method: property function_parens
			//this is a dumb and smart optimisation
			if (tokens[index+1].type == ParserToken::LPAREN) object_method();
			else
			{
				//assignment: property '=' expr
				expect(ParserToken::ASSIGN);

				//expr: string_literal | recursive string_function | '{' expr '} | lib_type'
				if (child == "type")
				{
					expect(ParserToken::STATIC, ParserToken::DYNAMIC);
					current_class->is_static = (current.type == ParserToken::STATIC);
					current_class->needs_rebuild += (!Util::file_exists(current_class->out_name.c_str())); 
				}
				else
				{
					expect(ParserToken::STRING_LITERAL, ParserToken::LCURLY, ParserToken::RECURSIVE);

					if (current.type == ParserToken::STRING_LITERAL)
					{
						current_class->set_property(current.in_line, child, current.value); //set current property to the string literal
					}
					else 
					{
						current_class->clear_property(current.in_line, child); //clear array
						if (current.type == ParserToken::LCURLY)
							array(); //start the array
						else if (current.type == ParserToken::RECURSIVE)
						{
							recursive();
						}
					}
				}
			}
			
			break;
		
		case ParserToken::SYSTEM:
		{
			/*system_call: system '(' string_literal ')'
			{
				if(system_allowed) {
					puts(string_literal);
					system(string_literal);
				}
			}*/
			string command = string_function().value;
			if (system_allowed)
			{
				std::cout << command << "\n";
				Util::system(command);
			}
		} break;

		case ParserToken::SEMICOLON: //ignored
			break;

		default:
			Util::error(current.in_line, "Did not expect " + token_names[current.type] + ".");
			break;
		}

		current = next(); // get next token
	}

	if (parser_exit) exit(1); //if there was a non-fatal error, exit. 
}

bool Parser::object_method()
{
	if (child == "build") //void Class.build(void);
	{
		void_function(); 
		current_class->build();				
	}
	else if (child == "clean") //void Class.build(void);
	{
		void_function(); 
		current_class->clean();				
	}
	else
	{
		return false; //if not any of those, it's a property
	}

	return true; //will go to the next token
}

void Parser::array()
{
	//this is an expr, continuing '{' expr '} but doesn't allow nested arrays.
	while (current.type != ParserToken::RCURLY)
	{
		expect(ParserToken::STRING_LITERAL, ParserToken::RECURSIVE, ParserToken::IDENTIFIER, ParserToken::COMMA, ParserToken::RCURLY);
		if (current.type == ParserToken::RECURSIVE)
		{
			recursive();
		}
		else if (current.type == ParserToken::STRING_LITERAL)
		{
			current_class->add_to_property(current.in_line, child, current.value);
		}
		else if (current.type == ParserToken::IDENTIFIER)
		{
			if (child == "libs")
			{
				if (is_defined(current.value))
					current_class->add_to_property(current.in_line, child,
							classes[current.value]->out_name);
				else
					Util::fatal_error(current.in_line, "\"" + current.value + "\" is not defined");
			}
			else
			{
				Util::fatal_error(current.in_line, "classes can only be included in the " highlight_var("libraries")
													" (or " highlight_var("libs") ") property.");
			}
		}
	}
}
