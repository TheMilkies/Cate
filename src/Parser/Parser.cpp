#include "Parser/Parser.hpp"
#include "Parser/Lexer.hpp"

#include "Class/Project.hpp"
#include "Class/Library.hpp"

using namespace Util;

extern string dir;
static robin_hood::unordered_set<string> opened_files;

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
	if (opened_files.find(file_name) != opened_files.end())
		fatal("Already built \"" + file_name + "\"");

#ifdef DEBUG
	cout << "compiling " << file_name << '\n';
#endif // DEBUG

	opened_files.insert(file_name);
	
	std::ifstream file(file_name);
	if (file.fail())
		command_error("Cannot open file \"" + file_name + "\"");
	
	yyFlexLexer* lexer = new yyFlexLexer(file, cout); //create the lexer
	tokens.reserve(128); //optimization
	classes.reserve(8);

	ParserToken temp;
	
	//Flex doesn't work in a way you might expect, so we make it easier to work with
	while (temp.type = (ParserTokenKind)lexer->yylex()) 
	{
		temp.value.clear(); //reset to save some memory
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
	file.close();

	if(parser_exit) exit(1);

	parse(); //start parsing
}

__attribute__((optimize("unroll-loops")))
Parser::~Parser()
{
	for(auto &c : classes)
		delete c.second; //free the pointers
}

void Parser::define()
{
	temp_type = current.type; //library or project

	expect(IDENTIFIER);
	string &identifier = current.value;

	if (is_defined(identifier))
		fatal("\"" + identifier + "\" was already defined");
	
	//this is technically a factory... oh well
	if (temp_type == PROJECT)
		classes[identifier] = new Project;
	else //library
		classes[identifier] = new Library;

	current_class = classes[identifier]; //set the pointer to the current class

	current_class->name = identifier; //set its name to the identifier
}

void Parser::parse()
{
	current = next(); //gets the first token (0)
	
	child.reserve(16);
	while (current.type != END) //END = end of file
	{
		switch (current.type)
		{
		case PROJECT:
			define();
			break;

		case LIBRARY:
			define();
			expect(LPAREN);
			current_class->is_static = expect_type(); //will set to static if static
			expect(RPAREN);
			break;

		case IDENTIFIER: {
			/*property: string_literal '.' string_literal*/
			auto& parent = current.value;

			if (!is_defined(parent))
				fatal("\"" + parent + "\" is not defined.");

			if (current_class->name != parent)
				current_class = classes[parent];

			expect(DOT); //will go automatically to DOT, no goto needed
		}

		case DOT:
			expect(IDENTIFIER);
			child = current.value;

			//object_method: property function_parens
			//this is a dumb and smart optimisation
			if (peek().type == LPAREN) object_method();
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
						//set current property to the string literal
						current_class->set_property(current.line, child, current.value); 
					}
					else 
					{
						if (current.type == LCURLY)
							array(); //start the array
						else if (current.type == RECURSIVE)
							files_recursive();
					}
				}
			}
			
			break;
		
		case SYSTEM:
			if (system_allowed)
				user_system(current.line, string_function().value);
			else
				skip(3);
			break;

		case SUBCATE: {
			string name = string_function().value;
		
			add_cate_ending(name);

			if(!file_exists(name.c_str()))
			{
				name = dir + '/' + name;
				if(!file_exists(name.c_str()))
					fatal("File \"" + name + "\" not found.");
			}
			
			//start the subcate instance
			Parser* sub = new Parser(name);
			delete sub;

		}	break;

		default:
			error(current.line, "Did not expect " + token_names[current.type] + ".");
			break;
		}

		current = next(); // get next token
	}

	if (parser_exit) exit(1); //if there was a non-fatal error, exit. 
}

bool Parser::special_case()
{
#define set_bool(x) current_class->x = expect_bool(); return true;
	if (child == "type")
	{
		current_class->is_static = expect_type();
		current_class->needs_rebuild += (!file_exists(current_class->out_name.c_str())); 
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