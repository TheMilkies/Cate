#include "Class.hpp"

extern int thread_count;

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

	if (object_files.size() != files.size())
		Util::warning("somehow the amount of object files is not equal to hte amount of source files in " + name);
	
	string command = "mkdir -p " + out_dir;
	Util::system(command);

	//create folder
	string path = out_name.substr(0, out_name.find_last_of('/')+1);
	if (!path.empty()) 
	{
		command = "mkdir -p " + path;
		Util::system(command);
	}
}

//this is for threads.
void Class::build_object(int i)
{
	if (files[i].empty()) //for some reason it wants to build an empty file, how about No?
		return;
	if (Util::get_modified_time(files[i].c_str()) < Util::get_modified_time(object_files[i].c_str())) //if doesn't need recompilation
		return;
	
	string command = command_template + files[i] + " -o " + object_files[i];
	Util::system(command); //will exit if it can't compile
	needs_rebuild = true;
}

void Class::build_objects()
{
	if (already_built) return;

	if (thread_count >= files.size())
		thread_count = files.size();

	if (include_paths.size() > 0) // 2% speed? yes please
	{
		for(auto &path : include_paths)
			all_include_paths += "-I" + path + ' ';
	}

	command_template.reserve(128);
	command_template = compiler + ' ' + flags + ' '+ all_include_paths + "-c "; //this is a nice optimization
	for (int i = 0; i < files.size(); i+=thread_count)
	{
		for (int j = 0; j < thread_count; j++)
		{
			if (i+j > files.size())
				break;
			
			threads.emplace_back(&Class::build_object, this, i+j);
		}
		for(auto &thread : threads)
		{
			thread.join();
		}
		
		threads.clear();
	}

	if (!needs_rebuild) //don't add libraries if you don't need to
		return;

	for(auto &lib : libraries)
	{
		//path check
		int position_of_last_slash = lib.find_last_of('/'); 
		string path = lib.substr(0, position_of_last_slash+1);

		if (!path.empty() && library_paths.find(path) == library_paths.end()) //if not there
			library_paths.insert(path);

		Util::remove_extention(lib);
		lib = lib.substr(position_of_last_slash+1 , lib.length()); //remove path from lib
		Util::replace_all(lib, "lib", ""); //THIS IS AN ISSUE
		
		all_libraries += "-l" + lib + ' ';
	}

	for(auto &path : library_paths)
	{
		all_library_paths += "-L" + path + ' ';
	}

	already_built = true;
}

void Class::clear_property(int line, string& property)
{
	if (property == "files")
		files.clear();
	else if (property == "libraries" || property == "libs")
		libraries.clear();
	else if (property == "includes" || property == "include_paths")
		include_paths.clear();
	else
		Util::error(line, "\"" + property + "\" cannot be set to an array");
}

void Class::add_to_property(int line, string& property, string value)
{
	if (property == "files")
		files.emplace_back(value);
	else if (property == "libraries" || property == "libs")
		libraries.emplace_back(value);
	else if (property == "includes" || property == "include_paths")
		include_paths.emplace_back(value);
}

void Class::set_property(int line, string& property, string& value)
{
	if (property == "name" || property == "out")
		out_name = value;
	else if (property == "flags")
		flags = value;
	else if (property == "compiler")
		compiler = value;
	else if (property == "final_flags" || property == "end_flags")
		final_flags = value;
	else if (property == "build_directory" || property == "object_folder")
		out_dir = value;
	else
		Util::error(line, "\"" + property + "\" cannot be set to a string");
}

void Class::check()
{
	if (already_built) return;

	if (parser_exit)
		Util::build_error(name, "of previous errors");

	if (files.empty())
		Util::build_error(name, "it has no files");

	if (compiler.empty())
		Util::build_error(name, "it has no compiler");

	if (out_name.empty())
		out_name = name;

	if (out_dir.empty())
		out_dir = "build";
}