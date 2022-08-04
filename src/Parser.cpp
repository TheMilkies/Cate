#include "Parser.hpp"

Parser::Parser(const char* file_name)
{
	//load yylex into tokens
	std::ifstream file(file_name);
	if (file.fail())
		Util::error("Cannot open file \"" + string(file_name) + "\"");
	
	FlexLexer* lexer = new yyFlexLexer(file, std::cout);
	tokens.reserve(100); //optimization

	ParserToken::ParserTokens type;
	string value;
	ParserToken temp;
	while (type = (ParserToken::ParserTokens)lexer->yylex()) //Flex doesn't work in a way you might expect, so we make it easier to work with
	{
		temp.type = type;
		temp.value = "";
		
		if (type == ParserToken::STRING_LITERAL || type == ParserToken::IDENTIFIER)
		{
			value = lexer->YYText();
			Util::remove_quotes(value);
			temp.value = value;
		}

		temp.in_line = lexer_line;
		tokens.emplace_back(temp);
	}
	
	delete lexer;
	parser_exit = lexer_exit; //parser will exit after all errors are printed
}

Parser::~Parser()
{
}

void Parser::define(ParserToken::ParserTokens type, string &identifier)
{
	if (is_defined(identifier))
		Util::error(current.in_line, "\"" + identifier + "\" was already defined");
	
	if (type == ParserToken::PROJECT)
		classes[identifier] = new Project;
	else
		classes[identifier] = new Library;

	classes[identifier]->name = identifier;
}

void Parser::parse()
{
	current = next();
	string parent, child;
	ParserToken::ParserTokens temp_type;
	while (current.type != ParserToken::END)
	{
		switch (current.type)
		{
		case ParserToken::PROJECT:
			temp_type = current.type;
			expect(ParserToken::IDENTIFIER);
			define(temp_type, current.value);
			break;
		case ParserToken::LIBRARY:
			expect(ParserToken::IDENTIFIER);
			define(temp_type, current.value);
			current_class = classes[current.value];
			current_class->is_library = true;
			expect(ParserToken::LPAREN);
			expect(ParserToken::STATIC, ParserToken::DYNAMIC);
			temp_type = current.type;
			current_class->is_static = (current.type == ParserToken::STATIC);
			expect(ParserToken::RPAREN);
			break;

		case ParserToken::IDENTIFIER:
			if (!is_defined(current.value))
				Util::error(current.in_line, "\"" + current.value + "\" is not defined.");

			parent = current.value;
			current_class = classes[parent];

			expect(ParserToken::DOT);
			expect(ParserToken::IDENTIFIER);
			child = current.value;
			if (child == "build")
			{
				expect(ParserToken::LPAREN);
				expect(ParserToken::RPAREN);
				current_class->build();
			}
			else
			{
				expect(ParserToken::ASSIGN);
				expect(ParserToken::STRING_LITERAL, ParserToken::LCURLY, ParserToken::RECURSIVE);
				if (current.type == ParserToken::LCURLY)
				{
					current_class->clear_property(current.in_line, child);
					array(child);
				}
				else if (current.type == ParserToken::RECURSIVE)
				{
					current_class->clear_property(current.in_line, child);
					recursive(child);
				}
				else
				{	
					current_class->set_property(current.in_line, child, current.value);
				}
			}
			
			break;
		
		case ParserToken::SYSTEM: {
			expect(ParserToken::LPAREN);
			expect(ParserToken::STRING_LITERAL);
			string command = current.value;
			expect(ParserToken::RPAREN);
			std::cout << command << "\n";
			system(command.c_str());
		}
			break;

		case ParserToken::SEMICOLON:
			break;

		default:
			Util::error(current.in_line, "Did not expect " + token_names[current.type] + ".");
			break;
		}

		//expect(ParserToken::SEMICOLON);

		current = next();
	}
	if (parser_exit)
		exit(1);
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
		Util::error(current.in_line, "Expected " + token_names[type] + "or " + token_names[type2] + " or " + token_names[type3] +
					" but got " + token_names[current.type]);
	}
}

void Parser::expect(ParserToken::ParserTokens type, ParserToken::ParserTokens type2, ParserToken::ParserTokens type3, ParserToken::ParserTokens type4)
{
	current = next();

	if (current.type != type && current.type != type2 && current.type != type3 && current.type != type4)
	{
		Util::error(current.in_line, "Expected " + token_names[type] + "or " + token_names[type2] + " or " + token_names[type3] +
					" or " + token_names[type4] + " but got " + token_names[current.type]);
	}
}

void Parser::array(string& child)
{
	while (current.type != ParserToken::RCURLY)
	{
		expect(ParserToken::STRING_LITERAL, ParserToken::RECURSIVE, ParserToken::COMMA, ParserToken::RCURLY);
		if (current.type == ParserToken::STRING_LITERAL)
		{
			current_class->add_to_property(current.in_line, child, current.value);
		}
		else if (current.type == ParserToken::RECURSIVE)
		{
			if (child == "libraries" || child == "libs")
				recursive(child, false); //don't keep path for libraries
			else
				recursive(child);
		}
	}
}

void Parser::recursive(string& child, bool keep_path)
{
	expect(ParserToken::LPAREN);
	expect(ParserToken::STRING_LITERAL);
	
	//wildcard stuff
	int location = current.value.find('*');

	if (location == string::npos) //if not found
		Util::error("Wildcard was not found in recursive");

	if (current.value[location+1] == '/')
		Util::error("Wildcard is not allowed in folders for now, might be in Cate_v2");
	
	string path = current.value.substr(0, location),
						extention = current.value.substr(location+1);

	if (path.find('*') != string::npos || extention.find('*') != string::npos) //if if more than one found
		Util::error("Multiple wildcards are not allowed");

	//check if directory exists
	if (!fs::is_directory(path))
		Util::error("Directory \"" + path + "\" doesn't exit");

	if (child == "libs" || child == "libraries")
	{
		if(current_class->library_paths.find(path) == current_class->library_paths.end())
			current_class->library_paths.insert(path);
	}
	
		
	for (auto &p : fs::recursive_directory_iterator(path))
    {
        if (p.path().extension() == extention)
			if (keep_path)
			{
				current_class->add_to_property(current.in_line, child, p.path().string());
			}
			else
			{
				current_class->add_to_property(current.in_line, child, p.path().stem().string());
			}
			
    }

	expect(ParserToken::RPAREN);
}