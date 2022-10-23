#include "Parser.hpp"
#define string_find(x, text) (x.find(text) != string::npos)

void Parser::recursive()
{
	if (child != "files")
		Util::fatal_error(current.line, "only the " highlight_var("files")
						" property can be set to result of recursive search.");
	
	current = string_function(); 

	if (current.value.empty()) //should NEVER happen
		Util::fatal_error(current.line, highlight_func("recursive()")
		" was given an empty string literal");
	
	//wildcard stuff
	int32_t location_of_wildcard = current.value.find('*');
	bool subrecursive = false;

	if (location_of_wildcard == string::npos) //if not found
		Util::error("Wildcard (*) was not found in " highlight_func("recursive()"));

	if (current.value[location_of_wildcard+1] == '/')
		Util::error(highlight_func("recursive()")
			" does not support folder recursion (f/*/*.c).");
	
	if (current.value[location_of_wildcard+1] == '*')
	{
		++location_of_wildcard;
		subrecursive = true;
	}
	
	string path = current.value.substr(0, location_of_wildcard - subrecursive), //extract path
				  extension = current.value.substr(location_of_wildcard+1); //extract extension
	
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
	
	Util::replace_all(path, " ", "\\ "); //for when your path has spaces, WINDOWS (mostly)

	if (!fs::is_directory(path)) //check if directory exists
	{
		if(!path.empty())
			Util::fatal_error(current.line, "Directory \"" + path + "\" doesn't exist");
	}

	if (string_find(path, '*') || string_find(extension, '*')) //if more than one found
		Util::fatal_error(current.line, "Multiple wildcards are not allowed");

	if (subrecursive)
	{
		if(path.empty()) path = ".";
		for (auto &p : fs::recursive_directory_iterator(path)) //iterate over the files
		{
			//add to files only if the files have the extension
			if (p.path().extension() == extension)
				current_class->files.emplace_back(p.path().string().c_str()+2); //skip ./
		}
	}
	else
	{
		for (auto &p : fs::directory_iterator(path)) //iterate over the files
		{
			//add to files only if the files have the extension
			if (p.path().extension() == extension)
				current_class->files.emplace_back(p.path().string());
		}
	}
}