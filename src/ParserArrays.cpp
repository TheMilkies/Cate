#include "Parser.hpp"

void Parser::array()
{
	if(child == "files") //last edge case
	{
		files_array();
	}
	else if(child == "libs" || child == "libraries")
	{
		library_array();
	}
	else if(child == "incs" || child == "includes" || child == "include_paths")
	{
		include_array();
	}
	else
	{
		Util::fatal_error(current.line, "\"" PURPLE + child + COLOR_RESET
			"\" cannot be set to an "
			"array or is not a valid property name");
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


void Parser::files_array()
{
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