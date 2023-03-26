#include "Parser/Parser.hpp"

using namespace Util;

#define sub_file_iterator(p) for (const auto& p : fs::recursive_directory_iterator(path))
#define file_iterator(p) for (const auto& p : fs::directory_iterator(path))

static struct
{
	string path;
	i32 location_of_wildcard = 0;
	bool subrecursive;
} rd; //recursive data

void Parser::recursive_setup()
{
	if(!match(STRING_LITERAL))
		current = string_function(); 

	auto& path = rd.path;
	auto& subrecursive = rd.subrecursive;
	auto& location_of_wildcard = rd.location_of_wildcard;

	string& argument = current.text;

	if (argument.empty()) //should NEVER happen
		fatal(hl_func("recursive()")
		" was given an empty string literal");

	location_of_wildcard = argument.find('*');
	subrecursive = false;

	if (location_of_wildcard == string::npos) //if not found
		fatal("Wildcard (*) was not found in " hl_func("recursive()"));

	if (argument[location_of_wildcard+1] == '/')
		fatal(hl_func("recursive()")
			" does not support folder recursion (f/*/*.c). Use subrecursive (f/**.c)");
	
	if (argument[location_of_wildcard+1] == '*')
	{
		++location_of_wildcard;
		subrecursive = true;
	}

	path = argument.substr(0, location_of_wildcard - subrecursive); //extract path
	
	replace_all(path, " ", "\\ "); //for when your path has spaces, WINDOWS (mostly)

	if (path.empty())
		path = "./";

	if (!fs::is_directory(path)) //check if directory exists
		fatal("Directory \"" + path + "\" doesn't exist");
}

string Parser::extension_recursive()
{
	recursive_setup();

	auto& path = rd.path;
	auto& subrecursive = rd.subrecursive;
	auto& files = current_class->files;

	string extension = current.text.substr(rd.location_of_wildcard+1); //extract extension
	
	if (extension.empty())
	{
		fatal(hl_func("recursive()")
				" was not given an extension to find.");
	}

	if (extension == ".*")
	{
		fatal(hl_func("recursive()")
		   " does not allow \"all file extensions\" (.*) recursion.");
	}

	if (string_find(path, '*') || string_find(extension, '*')) //if more than one found
		fatal("Multiple wildcards are not allowed");
	
	return extension;
}

void Parser::files_recursive()
{
	if (child != "files")
	{
		if(child == "libs" || child == "libraries")
			return library_recursive();
		else if(child == "incs" || child == "includes" || child == "include_paths")
			return include_recursive();

		fatal("only the " hl_var("files") ", " hl_var("includes") ", and " hl_var("libraries")
				" properties can be set to result of recursive search.");
	}
	
	string extension = extension_recursive();

	auto& path = rd.path;
	auto& subrecursive = rd.subrecursive;
	auto& files = current_class->files;

	if (subrecursive)
	{
		sub_file_iterator(p) //iterate over the files
		{
			//add to files only if the files have the extension
			if (p.path().extension() == extension)
				files.emplace_back(p.path().string());
		}
	}
	else
	{
		file_iterator(p)
		{
			//same here
			if (p.path().extension() == extension)
				files.emplace_back(p.path().string());
		}
	}
}

void Parser::include_recursive()
{
	recursive_setup();
	
	auto& path = rd.path;
	auto& subrecursive = rd.subrecursive;

	if (string_find(path, '*')) //if more than one found
		fatal("Multiple wildcards are not allowed");

	current_class->add_include(path);

	if(subrecursive)
	{
		sub_file_iterator(p)
		{
			if(fs::is_directory(p.path()))
				current_class->add_include(p.path().string());
		}
	}
	else
	{
		file_iterator(p)
		{
			if(fs::is_directory(p.path()))
				current_class->add_include(p.path().string());
		}
	}
}

void Parser::library_recursive()
{
	string extension = extension_recursive();

	auto& path = rd.path;
	auto& subrecursive = rd.subrecursive;

	if (subrecursive)
	{
		sub_file_iterator(p) //iterate over the files
		{
			//add to files only if the files have the extension
			if (p.path().extension() == extension)
				current_class->add_library(p.path().string());
		}
	}
	else
	{
		file_iterator(p)
		{
			//add to files only if the files have the extension
			if (p.path().extension() == extension)
				current_class->add_library(p.path().string());
		}
	}
}