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
	current_class->is_static = expect_type(); //if current token is static, set the type to static.
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
	//this is so jank yet so fast.
	if(child == "incs" || child == "includes" || child == "include_paths")
	{
		include_array();
		return;
	}
	else if(child == "libs" || child == "libraries")
	{
		library_array();
		return;
	}
	else if(child != "files") //last edge case
	{
		Util::fatal_error(current.line, "\"" PURPLE + child + COLOR_RESET "\" cannot be set to an array or is not a valid property name");
	}

	//now files
	vector<string>& current_property = current_class->files;
	static bool first_clear = true;
	if (first_clear)
	{
		first_clear = false;
		goto skip_clear_files;
	}
	current_class->files.clear();
	current_class->all_object_files.clear();
	current_class->object_files.clear();
	
skip_clear_files:
	//this is an expr, continuing '{' expr '} but doesn't allow nested arrays.
	while (current.type != RCURLY)
	{
		expect(STRING_LITERAL, RECURSIVE, COMMA, RCURLY);
		if (current.type == RECURSIVE)
		{
			recursive();
		}
		else if (current.type == STRING_LITERAL)
		{
			current_property.emplace_back(current.value);
		}
	}
}

void Parser::include_array()
{
	current_class->all_include_paths.clear();
	while (current.type != RCURLY)
	{
		expect(STRING_LITERAL, COMMA, RCURLY);
		if (current.type == STRING_LITERAL)
		{
			current_class->all_include_paths += "-I" + current.value + ' ';
		}
	}
}

void Parser::library_array()
{
	static bool first = true;
	if (first) //this saves a bit of time
	{
		first = false;
		goto skip_clear_libraries;
	}

	current_class->all_libraries.clear();
	current_class->all_library_paths.clear();

skip_clear_libraries:
	while (current.type != RCURLY)
	{
		expect(STRING_LITERAL, IDENTIFIER, COMMA, RCURLY);
		if (current.type == STRING_LITERAL)
		{
			current_class->add_library(current.value);
		}
		else if(current.type == IDENTIFIER)
		{
			if (is_defined(current.value))
				current_class->add_library(classes[current.value]->out_name);
			else
				Util::fatal_error(current.line, "\"" + current.value + "\" is not defined");
		}
		
	}
}

bool Parser::special_case()
{
#define set_bool(x) current_class->x = expect_bool(); return true;
	if (child == "type")
	{
		current_class->is_static = expect_type();
		current_class->needs_rebuild += (!Util::file_exists(current_class->out_name.c_str())); 
		return true;
	}
	else if (child == "threading")
	{
		set_bool(threading);
	}
	else if (child == "smolize" || child == "smol")
	{
		set_bool(smol);
	}
	else if (child == "link")
	{
		set_bool(link);
	}
#undef set_bool(x)

	return false;
}