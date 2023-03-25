#include "Parser/Parser.hpp"

using namespace Util;

void Parser::array()
{
	if(child == "files")
		files_array();
	else if(child == "libs" || child == "libraries")
		library_array();
	else if(child == "incs" || child == "includes" || child == "include_paths")
		include_array();
	else if(child == "defs" || child == "definitions" || child == "defines")
		definitions_array();
	else
	{
		fatal("\"" PURPLE + child + COLOR_RESET "\""
			  "cannot be set to an "
			  "array or is not a valid property name");
	}
}

void Parser::include_array()
{
	current_class->all_include_paths.clear();
	while (!match(RCURLY))
	{
		expect_string_recursive_array();
		if (match(STRING_LITERAL))
			current_class->add_include(current.text);
		else if (match(RECURSIVE))
			include_recursive();
	}
}

void Parser::definitions_array()
{
	auto& definitions = current_class->all_definitions;
	definitions.clear();
	while (!match(RCURLY))
	{
		expect_string_array();
		if (match(STRING_LITERAL))
			definitions += "-D" + current.text + ' ';
	}
}

void Parser::library_array()
{
	static bool first = true;
	if (first) //this saves a bit of time
	{
		current_class->all_libraries.clear();
		current_class->all_library_paths.clear();
	}
	else
	{
		first = false;
	}

	while (!match(RCURLY))
	{
		expect_library_recursive_array();
		auto& item = current.text;
		if (match(RECURSIVE) || string_find(current.text, '*'))
			library_recursive();
		else if (match(STRING_LITERAL))
			current_class->add_library(item);
		else if(match(IDENTIFIER))
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
	auto& files = current_class->files;
	static bool first_clear = true;
	if (!first_clear)
	{
		current_class->files.clear();
		current_class->all_object_files.clear();
		current_class->object_files.clear();
	}
	else first_clear = false;
	
	//this is an expr, continuing '{' expr '} but doesn't allow nested arrays.
	while (!match(RCURLY))
	{
		expect_string_recursive_array();
		if (match(RECURSIVE) || string_find(current.text, '*'))
			files_recursive();
		else if (match(STRING_LITERAL))
		{
			if(!file_exists(current.text.c_str()))
				warn(current.line, "File \"" + current.text + "\" was not found.");
			files.emplace_back(current.text);
		}
	}
}