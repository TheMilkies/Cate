#include "Class.hpp"

void Class::setup()
{
	check();
	for(auto &file : files)
	{
		string n = Util::replace_all_safe(file, "/", "_");
		n = (Util::remove_extention(n)) + ".o";
		n = out_dir + "/" + n;
		all_object_files += n + " ";
		object_files.emplace_back(n);
	}
}

void Class::build_objects()
{
	if (already_built) return;

	if (object_files.size() != files.size())
		Util::build_error(name, "somehow the amount of object files  is not equal to hte amount of source files");
	
	string command = "mkdir -p " + out_dir;
	Util::system(command);

	if (Util::get_modified_time(out_name.c_str()) == 0) //if file doesn't exist
		needs_rebuild = true;

	for (int i = 0; i < files.size(); i++)
	{
		if (Util::get_modified_time(files[i].c_str()) < Util::get_modified_time(object_files[i].c_str())) //if doesn't need recompilation
			continue;
		
		needs_rebuild = true;
		command = compiler + " -c " + files[i] + " -o " + object_files[i] + " " + flags;
		//std::cout << command << "\n"; //for debug
		Util::system(command); //will exit if it can't compile
	}

	if (!needs_rebuild) //don't add libraries if you don't need to
		return;

	for(auto lib : libraries)
	{
		Util::remove_extention(lib);
		Util::replace_all(lib, "lib", "");

		//path check
		int position_of_last_slash = lib.find_last_of('/'); 
		string path = lib.substr(0, position_of_last_slash+1);

		if (library_paths.find(path) == library_paths.end()) //if not there
			all_library_paths += "-L" + path + ' ';
		
		lib = lib.substr(position_of_last_slash+1 , lib.length()); //remove path from lib
		all_libraries += "-l" + lib + ' ';
	}

	for(auto &path : library_paths)
	{
		all_library_paths += "-L" + path + ' ';
	}

	already_built = true;
}

bool Class::clear_property(int line, string& property)
{
	if (property == "files")
		files.clear();
	else if (property == "libraries" || property == "libs")
		libraries.clear();
	else
		Util::error(line, "\"" + property + "\" cannot be set to an array");
	
	return true;
}

void Class::add_to_property(int line, string& property, string value)
{
	if (property == "files")
		files.emplace_back(value);
	if (property == "libraries" || property == "libs")
		libraries.emplace_back(value);
}

void Class::set_property(int line, string& property, string& value)
{
	if (property == "name" || property == "out")
		out_name = value;
	else if (property == "flags")
		flags = value;
	else if (property == "compiler")
		compiler = value;
	else if (property == "build_directory" || property == "object_folder")
		out_dir = value;
	else
		Util::error(line, "\"" + property + "\" cannot be set to a string");
}

void Class::check()
{
	if (parser_exit)
		Util::build_error(name , "of previous errors");

	if (out_name == "")
		out_name = name;

	if (compiler == "")
		Util::build_error(name, "it has no compiler");

	if (out_dir == "")
		out_dir = "build";

	if (files.empty())
		Util::build_error(name, "it has no files");
}