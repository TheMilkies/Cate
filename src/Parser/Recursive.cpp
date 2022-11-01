#include "Parser/Parser.hpp"
#define string_find(x, text) (x.find(text) != string::npos)

static struct
{
	string path;
	int32_t location_of_wildcard = 0;
	bool subrecursive;
} rd;

void Parser::recursive_setup()
{
	current = string_function(); 

	if (current.value.empty()) //should NEVER happen
		Util::fatal_error(current.line, highlight_func("recursive()")
		" was given an empty string literal");

	rd.location_of_wildcard = current.value.find('*');
	rd.subrecursive = false;

	if (rd.location_of_wildcard == string::npos) //if not found
		Util::error("Wildcard (*) was not found in " highlight_func("recursive()"));

	if (current.value[rd.location_of_wildcard+1] == '/')
		Util::error(highlight_func("recursive()")
			" does not support folder recursion (f/*/*.c). Use subrecursive (f/**.c)");
	
	if (current.value[rd.location_of_wildcard+1] == '*')
	{
		++rd.location_of_wildcard;
		rd.subrecursive = true;
	}

	rd.path = current.value.substr(0, rd.location_of_wildcard - rd.subrecursive); //extract path
	
	Util::replace_all(rd.path, " ", "\\ "); //for when your path has spaces, WINDOWS (mostly)

	if (!fs::is_directory(rd.path)) //check if directory exists
	{
		if(!rd.path.empty())
			Util::fatal_error(current.line, "Directory \"" + rd.path + "\" doesn't exist");
	}
}

void Parser::recursive()
{
	if(child == "incs" || child == "includes" || child == "include_paths")
		return include_recursive();

	if (child != "files")
		Util::fatal_error(current.line, "only the " highlight_var("files") " and " highlight_var("includes")
						" properties can be set to result of recursive search.");
	
	recursive_setup();

	string &path = rd.path;
	bool &subrecursive = rd.subrecursive;

	string extension = current.value.substr(rd.location_of_wildcard+1); //extract extension
	
	if (extension.empty())
	{
		Util::fatal_error(current.line, highlight_func("recursive()")
							" was not given an extension to find.");
	}

	if (extension == ".*")
	{
		Util::fatal_error(current.line, highlight_func("recursive()")
		   " does not allow \"all file extensions\" (.*) recursion.");
	}

	if (string_find(path, '*') || string_find(extension, '*')) //if more than one found
		Util::fatal_error(current.line, "Multiple wildcards are not allowed");

	if (subrecursive)
	{
		for (const auto &p : fs::recursive_directory_iterator(path)) //iterate over the files
		{
			//add to files only if the files have the extension
			if (p.path().extension() == extension)
				current_class->files.emplace_back(p.path().string());
		}
	}
	else
	{
		for (const auto &p : fs::directory_iterator(path)) //iterate over the files
		{
			//add to files only if the files have the extension
			if (p.path().extension() == extension)
				current_class->files.emplace_back(p.path().string());
		}
	}
}

void Parser::include_recursive()
{
	recursive_setup();
	
	string &path = rd.path;
	bool &subrecursive = rd.subrecursive;

	if (string_find(path, '*')) //if more than one foundallall
		Util::fatal_error(current.line, "Multiple wildcards are not allowed");

	current_class->all_include_paths += "-I" + path + ' ';

	if(subrecursive)
	{
		for (const auto& p : fs::recursive_directory_iterator(path))
		{
			if(fs::is_directory(p.path()))
				current_class->all_include_paths += "-I" + p.path().string() + ' ';
		}
	}
	else
	{
		for (const auto& p : fs::directory_iterator(path))
		{
			if(fs::is_directory(p.path()))
				current_class->all_include_paths += "-I" + p.path().string() + ' ';
		}
	}
}