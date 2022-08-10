#include "Parser.hpp"

/*
	why hello there!
	i know most of this is bad, i honestly don't care too much because it's my first project.
*/

Parser::Parser(const char* file_name)
{
	//load yylex into tokens
	std::ifstream file(file_name);
	if (file.fail())
		Util::command_error("Cannot open file \"" + string(file_name) + "\"");
	
	FlexLexer* lexer = new yyFlexLexer(file, std::cout);
	tokens.reserve(128); //optimization
	classes.reserve(8);

	ParserToken::ParserTokens type;
	string value;
	value.reserve(16);
	ParserToken temp;
	while (type = (ParserToken::ParserTokens)lexer->yylex()) //Flex doesn't work in a way you might expect, so we make it easier to work with
	{
		temp.type = type;
		temp.value = "";
		
		if (type == ParserToken::STRING_LITERAL)
		{
			value = lexer->YYText();
			Util::remove_quotes(value);
			temp.value = value;
		}
		else if (type == ParserToken::IDENTIFIER)
		{
			value = lexer->YYText();
			temp.value = value;
		}

		temp.in_line = lexer_line;
		tokens.push_back(temp);
	}
	
	delete lexer;
	parser_exit = lexer_exit; //parser will exit after all errors are printed

	parse();
}

Parser::~Parser()
{
	for(auto &c : classes)
		delete c.second; //free the pointers
}

void Parser::define(ParserToken::ParserTokens type, const string &identifier)
{
	if (is_defined(identifier))
		Util::fatal_error(current.in_line, "\"" + identifier + "\" was already defined");
	
	if (type == ParserToken::PROJECT)
		classes[identifier] = new Project;
	else
		classes[identifier] = new Library;

	classes[identifier]->name = identifier;
}

ParserToken Parser::function()
{
	expect(ParserToken::LPAREN);
	expect(ParserToken::STRING_LITERAL);
	ParserToken to_return = current;
	expect(ParserToken::RPAREN);
	return to_return;
}

void Parser::declare()
{
	temp_type = current.type;
	expect(ParserToken::IDENTIFIER);
	define(temp_type, current.value);
	current_class = classes[current.value];
}

void Parser::declare_library()
{
	declare();

	expect(ParserToken::LPAREN);
	expect(ParserToken::STATIC, ParserToken::DYNAMIC);

	temp_type = current.type;
	current_class->is_static = (current.type == ParserToken::STATIC);

	expect(ParserToken::RPAREN);
}

void Parser::parse()
{
	current = next();
	string parent; parent.reserve(16); child.reserve(16);
	while (current.type != ParserToken::END)
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
			parent = current.value;

			if (!is_defined(parent))
				Util::fatal_error(current.in_line, "\"" + parent + "\" is not defined.");

			if (current_class->name != parent)
				current_class = classes[parent];

			expect(ParserToken::DOT);
			expect(ParserToken::IDENTIFIER);
			child = current.value;
			
			if (child == "type")
			{
				expect(ParserToken::ASSIGN);
				expect(ParserToken::STATIC, ParserToken::DYNAMIC);
				current_class->is_static = (current.type == ParserToken::STATIC);
				current_class->needs_rebuild = true;
			}
			else if (object_method());
			else
			{
				expect(ParserToken::ASSIGN);
				expect(ParserToken::STRING_LITERAL, ParserToken::LCURLY, ParserToken::RECURSIVE);

				if (current.type == ParserToken::STRING_LITERAL)
				{
					current_class->set_property(current.in_line, child, current.value);
				}
				else if (current.type == ParserToken::LCURLY)
				{
					current_class->clear_property(current.in_line, child);
					array();
				}
				else
				{
					current_class->clear_property(current.in_line, child);
					recursive();
				}
			}
			
			break;
		
		case ParserToken::SYSTEM:
		{
			string command = function().value;
			if (system_allowed)
			{
				std::cout << command << "\n";
				system(command.c_str());
			}
		} break;

		case ParserToken::SEMICOLON:
			break;

		default:
			Util::error(current.in_line, "Did not expect " + token_names[current.type] + ".");
			break;
		}

		current = next();
	}

	if (parser_exit) exit(1);
}

bool Parser::object_method()
{
	if (child == "build")
	{
		expect(ParserToken::LPAREN);
		expect(ParserToken::RPAREN);
		if (current_class == nullptr)
			Util::build_error("Null", "It's null");
		current_class->build();				
	}
	else
	{
		return false; //if not any of those, it's a property
	}

	return true;
}

void Parser::expect(ParserToken::ParserTokens type)
{
	current = next();

	if (current.type != type)
	{
		Util::error(current.in_line, "Expected " + token_names[type] +
					" but got " + token_names[current.type]);
	}
}

void Parser::expect(ParserToken::ParserTokens type, ParserToken::ParserTokens type2)
{
	current = next();

	if (current.type != type && current.type != type2)
	{
		Util::error(current.in_line, "Expected " + token_names[type] + " or " + token_names[type2] +
					" but got " + token_names[current.type]);
	}
}

void Parser::expect(ParserToken::ParserTokens type, ParserToken::ParserTokens type2, ParserToken::ParserTokens type3)
{
	current = next();

	if (current.type != type && current.type != type2 && current.type != type3)
	{
		Util::error(current.in_line, "Expected " + token_names[type] + " or " + token_names[type2] + " or " + token_names[type3] +
					" but got " + token_names[current.type]);
	}
}

void Parser::expect(ParserToken::ParserTokens type, ParserToken::ParserTokens type2, ParserToken::ParserTokens type3, ParserToken::ParserTokens type4)
{
	current = next();

	if (current.type != type && current.type != type2 && current.type != type3 && current.type != type4)
	{
		Util::error(current.in_line, "Expected " + token_names[type] + " or " + token_names[type2] + " or " + token_names[type3] +
					" or " + token_names[type4] + " but got " + token_names[current.type]);
	}
}

void Parser::array()
{
	while (current.type != ParserToken::RCURLY)
	{
		expect(ParserToken::STRING_LITERAL, ParserToken::RECURSIVE, ParserToken::COMMA, ParserToken::RCURLY);
		if (current.type == ParserToken::RECURSIVE)
		{
			recursive();
		}
		else if (current.type == ParserToken::STRING_LITERAL)
		{
			current_class->add_to_property(current.in_line, child, current.value);
		}
	}
}

void Parser::recursive(const bool keep_path)
{
	if (child != "files")
		Util::fatal_error(current.in_line, "only files can be set to result of recursive search.");
	
	current = function(); //might look weird but actually makes this much easier.

	if (current.value.empty())
		Util::fatal_error(current.in_line, child + " has an empty string literal");
	
	//wildcard stuff
	int location = current.value.find('*');

	if (location == string::npos) //if not found
		Util::error("Wildcard was not found in recursive");

	if (current.value[location+1] == '/')
		Util::error("Wildcard is not allowed in folders for now, might be in Cate_v2");
	
	string path = current.value.substr(0, location), //extract path
				extension = current.value.substr(location+1); //extract extension

	if (path.find('*') != string::npos || extension.find('*') != string::npos) //if more than one found
		Util::error("Multiple wildcards are not allowed");

	Util::replace_all(path, " ", "\\ "); //for when your path has spaces

	//check if directory exists
	if (!fs::is_directory(path))
		Util::error(current.in_line, "Directory \"" + path + "\" doesn't exit");

	//in codegen, it's going to add these as `-L$THIS_PATH`
	if (child == "libs" || child == "libraries")
		if(current_class->library_paths.find(path) == current_class->library_paths.end())
			current_class->library_paths.insert(path);

	for (auto &p : fs::directory_iterator(path))
    {
        if (p.path().extension() == extension)
			if (keep_path)
				current_class->add_to_property(current.in_line, child, p.path().string());
			else
				current_class->add_to_property(current.in_line, child, p.path().stem().string());
    }
}