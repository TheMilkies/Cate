#include "Parser/Parser.hpp"
#include "Parser/Lexer.hpp"

#include "Class/Project.hpp"
#include "Class/Library.hpp"
#include "Class/Global.hpp"
#include <fstream>
#include <memory>

using namespace Util;
extern string default_directory;
inline vector<string> opened_files;
inline std::vector<std::unique_ptr<Class>> classes;

bool was_file_opened(string_view file_name) {
	for (auto &name : opened_files)
	{
		if(name == file_name)
			return true;
	}
	return false;
}

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
	opened_files.reserve(8);
	if (was_file_opened(file_name))
		fatal("Already opened \"" + file_name + "\"");

#ifdef DEBUG
	cout << "parsing " << file_name << '\n';
#endif // DEBUG

	opened_files.emplace_back(file_name);
	
	std::ifstream file(file_name);
	if (file.fail())
		command_error("Cannot open file \"" + file_name + "\"");
	
	yyFlexLexer* lexer = new yyFlexLexer(file, cout); //create the lexer
	tokens.reserve(128); //optimization
	classes.reserve(8);

	ParserToken temp(END);
	//Flex doesn't work in a way you might expect, so we make it easier to work with
	while (temp.type = (ParserTokenKind)lexer->yylex()) 
	{
		temp.text.clear(); //reset to save some memory
		temp.line = lexer_line; //defined in lexer.l
		
		if (temp.type == STRING_LITERAL)
		{
			//string literals have the quotes (example: "string") so we need to remove them
			temp.text = lexer->YYText()+1;
			temp.text.pop_back();
		}
		else if (temp.type == IDENTIFIER)
		{
			//identifiers are just kept as themselves
			temp.text = lexer->YYText();
		}

		tokens.push_back(temp); //add the token
	}
	
	delete lexer; //delete the monstrosity and free its memory
	file.close();

	if(errors_exist) exit(1);

	next(); //gets the first token (0)
	parse(); //start parsing
}

__attribute__((optimize("unroll-loops")))
Parser::~Parser()
{
}

void Parser::define()
{
	bool is_project = current.type == PROJECT; //library or project

	expect(IDENTIFIER);
	string &identifier = current.text;

	if (get_class(identifier))
		fatal("\"" + identifier + "\" was already defined");
	
	//this is technically a factory... oh well
	if (is_project)
		current_class = classes.emplace_back(new Project(identifier)).get();
	else //library
		current_class = classes.emplace_back(new Library(identifier)).get();
}

void Parser::parse()
{	
	child.reserve(16);
	while (!match(END)) //END = end of file
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
			if(global())
				break;

			/*property: parent=string_literal '.' child=string_literal*/
			auto parent = current.text;
			auto other_class = get_class(parent);

			if (!other_class)
				fatal("\"" + parent + "\" is not defined.");

			if (current_class->name != parent)
				current_class = other_class;

			expect(DOT); //will go automatically to DOT, no goto needed
		}

		case DOT:
			if(!current_class)
				fatal("No object was selected.\n"
				"if you're trying to use a global variable, it's without the dot.");

			expect(IDENTIFIER);
			child = current.text;

			//object_method: property function_parens
			//this is a dumb and smart optimisation
			if (peek() == LPAREN)
			{
				object_method();
				break;
			}

			//assignment: property '=' expr
			expect(ASSIGN);
			if(special_case()) break; // handled there

			//expr: string_literal | recursive string_function | '{' expr '} | lib_type'
			expect(STRING_LITERAL, LCURLY, RECURSIVE);
			if (match(STRING_LITERAL))
			{
				//set current property to the string literal
				current_class->set_property(current.line, child, current.text); 
			}
			else
			{
				if (match(LCURLY))
					array(); //start the array
				else if (match(RECURSIVE))
					files_recursive();
			}
			break;
		
		case SYSTEM:
			if (flag(system_blocked))
				skip(3);
			else
				user_system(current.line, string_function().text);
			break;

		case RECURSIVE:
			warn(current.line, 
				hl_func("recursive()")
				" is outside of an assignment.");
			string_function();
			break;

		case SUBCATE: {
			string name = string_function().text;
			add_cate_ending_to(name);

			if(!file_exists(name.c_str()))
			{
				name = default_directory + '/' + name;
				if(!file_exists(name.c_str()))
					fatal("File \"" + name + "\" not found.");
			}

			if(flag(dry_run))
				cout << "cate " << name << std::endl;
			
			//start the subcate instance
			Parser sub(name);
		}	break;

		default:
			error(current.line, "Did not expect " + token_names[current.type] + ".");
			break;
		}

		next(); // get next token
	}

	if (errors_exist) exit(1); //if there was a non-fatal error, exit. 
}

#define set_string(x) {expect(STRING_LITERAL);\
						global_values.x = current.text;}
bool Parser::global()
{
	if(peek() != ASSIGN)
		return false;
	child = current.text;
	expect(ASSIGN);

	if (child == "cc" || child == "compiler")
		set_string(compiler)
	else if (child == "std" || child == "standard")
		set_string(standard)
	else if (child == "obj_dir"		   ||
			 child == "build_dir" 	   ||
			 child == "object_folder"   ||
			 child == "build_directory")
		set_string(object_dir)
#define set_bool(x) {global_values.x = expect_bool();}
	else if (child == "threading")
		set_bool(threading)
	else if (child == "smol" || child == "smolize")
		set_bool(smol)
	else
		fatal("unknown global variable \"" + child + "\"");

	return true;
#undef set_bool
}

Class *Parser::get_class(std::string_view name)
{
	for (auto &c : classes)
	{
		if(c->name == name)
			return c.get();
	}
	
	return NULL;
}

#define set_bool(x) current_class->x = expect_bool();
bool Parser::special_case()
{
	if (child == "type")
		current_class->set_type(current.line, expect_type()); 
	else if (child == "threading" || child == "thread")
		set_bool(threading)
	else if (child == "smol" || child == "smolize")
		set_bool(smol)
	else if (child == "link")
		set_bool(link)
	else return false;

	return true;
}
#undef set_bool