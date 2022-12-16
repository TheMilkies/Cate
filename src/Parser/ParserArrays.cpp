#include "Parser/Parser.hpp"

using namespace Util;

void Parser::array()
{
	if(child == "files")
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
	else if(child == "defs" || child == "definitions" || child == "defines")
	{
		definitions_array();
	}
	else
	{
		fatal("\"" PURPLE + child + COLOR_RESET
			"\" cannot be set to an "
			"array or is not a valid property name");
	}
}

void Parser::include_array()
{
	current_class->all_include_paths.clear();
	while (current.type != RCURLY)
	{
		expect_string_recursive_array();
		if (current.type == STRING_LITERAL)
			current_class->add_include(current.value);
		else if (current.type == RECURSIVE)
			include_recursive();
	}
}

void Parser::definitions_array()
{
	auto& definitions = current_class->all_definitions;
	definitions.clear();
	while (current.type != RCURLY)
	{
		expect_string_array();
		if (current.type == STRING_LITERAL)
			definitions += "-D" + current.value + ' ';
	}
}

void Parser::library_array()
{
	static bool first = true;
	if (!first) //this saves a bit of time
	{
		current_class->all_libraries.clear();
		current_class->all_library_paths.clear();
	}
	else
	{
		first = false;
	}

	while (current.type != RCURLY)
	{
		expect(STRING_LITERAL, IDENTIFIER, COMMA, RCURLY);
		auto& item = current.value;
		if (current.type == STRING_LITERAL)
		{
			current_class->add_library(item);
		}
		else if(current.type == IDENTIFIER)
		{
			if (is_defined(item))
				current_class->add_library(classes[item]->out_name);
			else
				fatal("\"" + item + "\" is not defined");
		}
	}
}

void Parser::files_array()
{
	//now files
	auto& current_property = current_class->files;
	static bool first_clear = true;
	if (!first_clear)
	{
		current_class->files.clear();
		current_class->all_object_files.clear();
		current_class->object_files.clear();
	}
	else
	{
		first_clear = false;
	}
	
	//this is an expr, continuing '{' expr '} but doesn't allow nested arrays.
	while (current.type != RCURLY)
	{
		expect_string_recursive_array();
		if (current.type == RECURSIVE || string_find(current.value, '*'))
			files_recursive();
		else if (current.type == STRING_LITERAL)
			current_property.emplace_back(current.value);
	}
}