#include "Parser/Parser.hpp"

using namespace Util;

#define sub_file_iterator(p) for (const auto& p : fs::recursive_directory_iterator(path))
#define file_iterator(p) for (const auto& p : fs::directory_iterator(path))

static struct
{
	string path;
	int32_t location_of_wildcard = 0;
	bool subrecursive;
} rd; //recursive data

void Parser::recursive_setup()
{
	current = string_function(); 

	string &path = rd.path;
	bool &subrecursive = rd.subrecursive;
	int32_t &location_of_wildcard = rd.location_of_wildcard;

	if (current.value.empty()) //should NEVER happen
		fatal_error(current.line, hl_func("recursive()")
		" was given an empty string literal");

	location_of_wildcard = current.value.find('*');
	subrecursive = false;

	if (location_of_wildcard == string::npos) //if not found
		fatal_error(current.line, "Wildcard (*) was not found in " hl_func("recursive()"));

	if (current.value[location_of_wildcard+1] == '/')
		fatal_error(current.line, hl_func("recursive()")
			" does not support folder recursion (f/*/*.c). Use subrecursive (f/**.c)");
	
	if (current.value[location_of_wildcard+1] == '*')
	{
		++location_of_wildcard;
		subrecursive = true;
	}

	path = current.value.substr(0, location_of_wildcard - subrecursive); //extract path
	
	replace_all(path, " ", "\\ "); //for when your path has spaces, WINDOWS (mostly)

	if (!fs::is_directory(path)) //check if directory exists
	{
		fatal_error(current.line, "Directory \"" + path + "\" doesn't exist");
	}
}

void Parser::recursive()
{
	if(child == "incs" || child == "includes" || child == "include_paths")
		return include_recursive();

	if (child != "files")
		fatal_error(current.line, "only the " hl_var("files") " and " hl_var("includes")
				" properties can be set to result of recursive search.");
	
	recursive_setup();

	string &path = rd.path;
	bool &subrecursive = rd.subrecursive;

	vector<string>& files = current_class->files;

	string extension = current.value.substr(rd.location_of_wildcard+1); //extract extension
	
	if (extension.empty())
	{
		fatal_error(current.line, hl_func("recursive()")
				" was not given an extension to find.");
	}

	if (extension == ".*")
	{
		fatal_error(current.line, hl_func("recursive()")
		   " does not allow \"all file extensions\" (.*) recursion.");
	}

	if (string_find(path, '*') || string_find(extension, '*')) //if more than one found
		fatal_error(current.line, "Multiple wildcards are not allowed");

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
			//add to files only if the files have the extension
			if (p.path().extension() == extension)
				files.emplace_back(p.path().string());
		}
	}
}

void Parser::include_recursive()
{
	recursive_setup();
	
	string &path = rd.path;
	bool &subrecursive = rd.subrecursive;

	if (string_find(path, '*')) //if more than one found
		fatal_error(current.line, "Multiple wildcards are not allowed");

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