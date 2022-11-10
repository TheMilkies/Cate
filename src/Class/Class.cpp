#include "Class/Class.hpp"

extern int32_t thread_count;

extern bool force_rebuild, force_smol;

Class::Class()
{
	//these should be enough for most small/medium-sized projects
	name.reserve(32);
	files.reserve(32);
	object_files.reserve(32);
	library_paths.reserve(32);
	all_libraries.reserve(128);
	threads.reserve(thread_count * 4);

	all_include_paths.reserve(256);
	all_library_paths.reserve(128);
	all_object_files.reserve(512);
	all_definitions.reserve(256);

	compiler.reserve(16);
	final_flags.reserve(64);
	out_name.reserve(32);
	flags.reserve(256);
	out_dir.reserve(64);
}

void Class::setup()
{
	//if (already_built) return; //i don't think this is needed
	check();

	static bool bypass_force = false; //for when making static and dynamic
	if(force_rebuild)
	{
		if(!bypass_force)
		{
			clean();
			bypass_force = true;
		}
	}

	//calling object_setup() on another thread is a bit faster
	std::thread object_thread(&Class::object_setup, this);

	//create output file's folder if doesn't already exist.
	create_directories();

	//wait until these finish
	object_thread.join();
}

void Class::object_setup()
{
	for(auto file : files)
	{
		Util::replace_all(file, "../", "back_"); //  "../" -> "back_"
		//Util::replace_all(file, "./", ""); // "./" -> ""
		Util::replace_all(file, "/", "_"); // "/" -> "_"
		file = (Util::remove_extension(file)) + OBJ_EXTENSION; //replace the extension with .o
		file = out_dir + "/" + file; // folder/object.o
		all_object_files += file + " "; //used in final build process, all are linked
		object_files.emplace_back(file); //for build
	}
}
//this is for threads.
void Class::build_object(int32_t i)
{
	Util::system(command_template + files[i] + " -o " + object_files[i]); //will exit if it can't compile
}

void Class::build_objects()
{
	//if (already_built) return; //i don't think this is needed

	if (thread_count >= files.size()) //this is very important.
		thread_count  = files.size();

	command_template.reserve(512);
	command_template = compiler + ' ' + flags + ' ' + all_definitions + all_include_paths + "-c "; //this is a nice optimization
	for (int32_t i = 0; i < files.size(); i+=thread_count)
	{
		for (int32_t j = 0; j < thread_count; ++j)
		{
			int32_t current = i+j;
			if (current > files.size()) break; //current file index check
			if (files[current].empty() || newer_than(files[current], object_files[current])) //if doesn't need recompilation
				continue;

			threads.emplace_back(&Class::build_object, this, current); //make thread build the object file
			needs_rebuild = true;
		}

		for(auto &thread : threads)
		{
			if (thread.joinable())
				thread.join(); //make sure the main thread waits until they finish.
		}
		
		threads.clear(); //clear them so we won't run them again.
	}

	//set by the threads, don't add libraries if you don't need to
	if (!needs_rebuild) return;

	already_built = true;
}

void Class::add_library(string& lib)
{
	//path check
	int32_t position_of_last_slash = lib.find_last_of('/'); 
	string path = lib.substr(0, position_of_last_slash+1);

	if (!path.empty() && library_paths.find(path) == library_paths.end()) //if not in library paths, add it
	{
		library_paths.insert(path);
		all_libraries += "-L" + path + ' ';
	}

	//check if not static/local
	if (!Util::ends_with(lib, ".a") && !Util::ends_with(lib, ".lib"))
	{
		Util::remove_extension(lib);
		lib = lib.substr(position_of_last_slash+1 , lib.length()); //remove path from lib
		Util::replace_all(lib, "lib", ""); //remove the lib part.
		all_libraries += "-l";
	}

	all_libraries += lib + ' ';
}

//self explanitories, i cry every time C++ doesn't have switch for strings

void Class::set_property(int32_t line, string& property, string& value)
{
	if (property == "out")
		out_name = value;
	else if (property == "flags")
		flags = value;
	else if (property == "compiler")
		compiler = value;
	else if (property == "final_flags" || property == "end_flags")
		final_flags = value;
	else if (property == "build_directory" ||
			 property == "object_folder"   ||
			 property == "obj_dir"		   ||
			 property == "build_dir")
		out_dir = value;
	else if (property == "standard" ||
			 property == "std")
		standard = value;
	else
		Util::error(line, "\"" PURPLE + property + COLOR_RESET "\" cannot be set to a string or is not a valid property name");
}

void Class::check()
{
	if (parser_exit)
	{
		Util::build_error(name, "of previous errors");
		exit(1);
	}

	if (files.empty())
		Util::build_error(name, "it has no files");

	if (compiler.empty())
		compiler = "cc";

	if (out_name.empty())
		generate_name();

	if (out_dir.empty())
	{
		//automation
		if (fs::is_directory("obj"))
			out_dir = "obj";
		else
			out_dir = "build";
	}

	if (threading)
		flags += " -pthread ";

	if (smol)
		flags += " -ffunction-sections -fdata-sections -Wl,--gc-sections -fno-ident -fomit-frame-pointer"
				 " -fmerge-all-constants -Wl,--build-id=none ";

	if (!standard.empty())
		flags += " -std=" + standard + " ";
	
	//automation
	if (all_include_paths.empty() && fs::is_directory("include"))
		all_include_paths = " -Iinclude ";
}

void Class::smolize()
{
	if(!smol && !force_smol) return;
	Util::system("strip -S --strip-unneeded --remove-section=.note.gnu.gold-version "
			"--remove-section=.comment --remove-section=.note --remove-section=.note.gnu.build-id --remove-section=.note.ABI-tag "
			+ out_name);
}

void Class::create_directories()
{
	Util::create_folder(out_dir.c_str());
	string path = out_name.substr(0, out_name.find_last_of('/')+1);
	
	if (!path.empty() && path != "./")
		Util::create_folder(path.c_str());
}