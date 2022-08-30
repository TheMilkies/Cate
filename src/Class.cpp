#include "Class.hpp"

extern int32_t thread_count;

void Class::setup()
{
	if (already_built) return;
	check();

	for(auto file : files)
	{
		Util::replace_all(file, "../", "back_"); //  "../" -> "back_"
		Util::replace_all(file, "/", "_"); // "/" -> "_"
		file = (Util::remove_extension(file)) + ".o"; //replace the extension with .o
		file = out_dir + "/" + file; // folder/object.o
		all_object_files += file + " "; //used in final build process, all are linked
		object_files.emplace_back(file); //for build
	}

	//useless debug line
	/*if (object_files.size() != files.size())
		Util::warning("somehow the amount of object files is not equal to hte amount of source files in " + name);*/ 
	
	//create build folder if it doesn't exist
	Util::create_folder(out_dir.c_str());

	//create output file's folder if doesn't already exist.
	string path = out_name.substr(0, out_name.find_last_of('/')+1);
	if (!path.empty())
	{
		Util::create_folder(path.c_str());
	}
}

//this is for threads.
void Class::build_object(int32_t i)
{
	if (files[i].empty()) //for some reason it wants to build an empty file, how about No?
		return;
	
	string command = command_template + files[i] + " -o " + object_files[i];
	Util::system(command); //will exit if it can't compile
	needs_rebuild = true;
}

void Class::build_objects()
{
	if (already_built) return; 

	if (thread_count >= files.size()) //this is very important.
		thread_count = files.size();

	if (!include_paths.empty())
	{
		for(auto &path : include_paths)
			all_include_paths += "-I" + path + ' ';
	}

	command_template.reserve(128);
	command_template = compiler + ' ' + flags + ' '+ all_include_paths + "-c "; //this is a nice optimization
	for (int32_t i = 0; i < files.size(); i+=thread_count)
	{
		for (int32_t j = 0; j < thread_count; j++)
		{
			int32_t current = i+j;
			if (current > files.size()) break; //current file index check
			if (Util::get_modified_time(files[current].c_str()) < Util::get_modified_time(object_files[current].c_str())) //if doesn't need recompilation
				continue;

			threads.emplace_back(&Class::build_object, this, current); //make thread build the object file
		}

		for(auto &thread : threads)
		{
			thread.join(); //make sure the main thread waits until they finish.
		}
		
		threads.clear(); //clear them so we won't run them again.
	}

	 //set by the threads, don't add libraries if you don't need to
	if (!needs_rebuild) return;

	for(auto &lib : libraries)
	{
		//path check
		int32_t position_of_last_slash = lib.find_last_of('/'); 
		string path = lib.substr(0, position_of_last_slash+1);

		if (!path.empty() && library_paths.find(path) == library_paths.end()) //if not in library paths, add it
			library_paths.insert(path);

		//check if static
		if (!Util::ends_with(lib, ".a") && !Util::ends_with(lib, ".lib"))
		{
			Util::remove_extension(lib);
			lib = lib.substr(position_of_last_slash+1 , lib.length()); //remove path from lib
			Util::replace_all(lib, "lib", ""); //remove the lib part.
			all_libraries += "-l";
		}

		all_libraries += lib + ' ';
	}

	//can be rewritten but it's faster like this...?
	for(auto &path : library_paths)
	{
		all_library_paths += "-L" + path + ' ';
	}

	already_built = true;
}

//self explanitories
void Class::clear_property(int32_t line, string& property)
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

void Class::add_to_property(int32_t line, string_view property, string_view value)
{
	if (property == "files")
		files.emplace_back(value);
	else if (property == "libraries" || property == "libs")
		libraries.emplace_back(value);
	else if (property == "includes" || property == "include_paths")
		include_paths.emplace_back(value);
}

void Class::set_property(int32_t line, string& property, string& value)
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
	if (parser_exit)
		Util::build_error(name, "of previous errors");

	if (files.empty())
		Util::build_error(name, "it has no files");

	if (compiler.empty())
		Util::build_error(name, "it has no compiler");

	if (out_name.empty())
	#ifdef __WIN32
		out_name = name + ".exe";
	#else
		out_name = name;
	#endif // __WIN32

	if (out_dir.empty())
		out_dir = "build";
}