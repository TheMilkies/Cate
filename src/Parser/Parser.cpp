#include "Parser/Parser.hpp"
#include "Parser/Lexer.hpp"

#include "Class/Project.hpp"
#include "Class/Library.hpp"
#include "Class/Global.hpp"
#include <fstream>


using namespace Util;

extern string default_directory;
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
	cout << "parsing " << file_name << '\n';
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
	bool is_project = current.type == PROJECT; //library or project

	expect(IDENTIFIER);
	string &identifier = current.text;

	if (is_defined(identifier))
		fatal("\"" + identifier + "\" was already defined");
	
	//this is technically a factory... oh well
	if (is_project)
		classes[identifier] = new Project(identifier);
	else //library
		classes[identifier] = new Library(identifier);

	current_class = classes[identifier]; //set the pointer to the current class
}

void Parser::parse()
{
	next(); //gets the first token (0)
	
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
			if(global()) break;
			/*property: string_literal '.' string_literal*/
			auto& parent = current.text;

			if (!is_defined(parent))
				fatal("\"" + parent + "\" is not defined.");

			if (current_class->name != parent)
				current_class = classes[parent];

			expect(DOT); //will go automatically to DOT, no goto needed
		}

		case DOT:
			if(current_class == nullptr)
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
			if (system_blocked)
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

#define set_string(x) {expect(STRING_LITERAL); global_values.x = current.text;}
bool Parser::global()
{
	child = current.text;
	switch (current.text[0]) //fast tm
	{
	case 'b':case 'c':case 't':case 's':case 'o': break;
	default: return false; break;
	}
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

	else return false;
#undef set_bool
	return true;
}

#define set_bool(x) current_class->x = expect_bool();
bool Parser::special_case()
{
	switch (child[0]) //fast tm
	{
	case 'l':case 't':case 's': break;
	default: return false; break;
	}
	
	if (child == "type")
		current_class->set_type(current.line, expect_type()); 
	else if (child == "threading")
		set_bool(threading)
	else if (child == "smol" || child == "smolize")
		set_bool(smol)
	else if (child == "link")
		set_bool(link)
	else return false;

	return true;
}
#undef set_bool