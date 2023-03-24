#include "Class/Class.hpp"
#include "Util.hpp"
#include "Class/Global.hpp"

extern int32_t thread_count;

extern bool force_rebuild, force_smol, dont_ask_install;

Class::Class(std::string_view ident): name(ident),
	object_dir(global_values.object_dir),
	threading (global_values.threading),
	smol 	  (global_values.smol),
	compiler  (global_values.compiler),
	standard  (global_values.standard)
{
	//these should be enough for most small/medium-sized projects
	files.reserve(32);
	object_files.reserve(32);
	loaded_library_paths.reserve(32);
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
	object_dir.reserve(64);
}

void Class::setup()
{
	check();

	static bool bypass_force = false; //for when making static and dynamic
	if(force_rebuild && !bypass_force)
	{
		clean();
		bypass_force = true;
	}

	//calling setup_objects() on another thread is a bit faster
	std::thread object_thread(&Class::setup_objects, this);

	//create output file's folder if doesn't already exist.
	create_directories();

	//wait until these finish
	object_thread.join();
}

void Class::setup_objects()
{
	for(auto file : files)
	{
		Util::replace_all(file, "../", "back_"); //  "../" -> "back_"
		Util::replace_all(file, "/", "_"); // "/" -> "_"
		file = (Util::remove_extension(file)) + OBJ_EXTENSION; //replace the extension with .o
		file = object_dir + "/" + file; // folder/object.o
		all_object_files += file + " "; //used in final build process, all are linked
		object_files.emplace_back(file); //for build
	}
}

void Class::install()
{
#ifdef __WIN32
	Util::warn(0, "Installing is not supported in Windows Cate.");
	return;
#endif // __WIN32

	if(out_name.empty()) generate_name();
	if(!Util::file_exists(out_name.c_str())) return;

	string install_path_name = get_install_path();
	if(!newer_than(install_path_name, out_name)) return;

	if(!ask_to_install()) return;

	Util::check_root();
	string command = "cp -f " + out_name + " " + install_path_name;
	Util::system(command);
	
	cout << GREEN "Installed \"" << name << "\"" COLOR_RESET "\n";
}

string Class::get_stripped_name()
{
	if(out_name.empty()) generate_name();

	int32_t position_of_slash = out_name.find_last_of('/');
	if(position_of_slash == string::npos) return out_name;

	return out_name.substr(position_of_slash, out_name.length());
}

// this is for threads.
void Class::build_object(int32_t i)
{
	Util::system(
		command_template + object_files[i] + " " + files[i]
	); //will exit if it can't compile
}

void Class::build_objects()
{
	//if (already_built) return; //i don't think this is needed

	if (thread_count >= files.size()) //this is very important.
		thread_count  = files.size();

	command_template.reserve(512);
	command_template = compiler + ' ' + flags + ' ' + all_definitions + all_include_paths + "-c -o"; //this is a nice optimization
	for (int32_t i = 0; i < files.size(); i+=thread_count)
	{
		for (int32_t j = 0; j < thread_count; ++j)
		{
			int32_t current = i+j;
			if (current > files.size()) break; //current file index check
			if (files[current].empty() || newer_than(files[current], object_files[current])) //if doesn't need recompilation
				continue;

			threads.emplace_back(&Class::build_object, this, current); //make thread build the object file
			needs_link = true;
		}

		for(auto &thread : threads)
		{
			if (thread.joinable())
				thread.join(); //make sure the main thread waits until they finish.
		}
		
		threads.clear(); //clear them so we won't run them again.
	}

#ifdef DEBUG
	cout << "finished " << name << '\n';
#endif // DEBUG

	already_built = true;
}

void Class::add_library(string& lib)
{
	//path check
	int32_t position_of_last_slash = lib.find_last_of('/'); 
	string path = lib.substr(0, position_of_last_slash+1);

	if (!path.empty() && loaded_library_paths.find(path) == loaded_library_paths.end()) //if not in library paths, add it
	{
		loaded_library_paths.insert(path);
		all_libraries += "-L" + path + ' ';
	}

	//check if not static/local
	if (!Util::ends_with(lib, ".a") && !Util::ends_with(lib, ".lib"))
	{
		Util::remove_extension(lib);
		lib = lib.substr(position_of_last_slash+1 , lib.length()); //remove path from lib
		lib = lib.substr(lib.find_first_not_of("lib"), lib.length());
		all_libraries += "-l";
	}

	all_libraries += lib + ' ';
}

//self explanitories, i cry every time C++ doesn't have switch for strings

void Class::set_property(int32_t line, string& property, string& value)
{
	if (property == "cc" || property == "compiler")
		compiler = value;
	else if (property == "flags")
		flags = value;
	else if (property == "out")
		out_name = value;
	else if (property == "final_flags" || property == "end_flags")
		final_flags = value;
	else if (property == "obj_dir"		   ||
			 property == "build_dir" 	   ||
			 property == "object_folder"   ||
			 property == "build_directory")
		object_dir = value;
	else if (property == "std" || property == "standard")
		standard = value;
	else
		Util::error(line, "\"" PURPLE + property + COLOR_RESET "\" cannot be set to a string or is not a valid property name");
}

void Class::build_error(string_view problem)
{
	cerr << ERROR ": Cannot build \"" << name
		<< "\" because " << problem << "\nTerminating.\n";
	exit(1);
}

void Class::check()
{
	using Util::file_exists;

	if (parser_exit)
		build_error("of previous errors");

	if (files.empty())
		build_error("it has no files");

	if (out_name.empty())
		generate_name();

	if (!file_exists(out_name.c_str()))
		needs_link = true;

	if (threading)
		flags += " -pthread ";

	if (smol)
		flags += " -ffunction-sections -fdata-sections -Wl,--gc-sections -fno-ident -fomit-frame-pointer"
				 " -fmerge-all-constants -Wl,--build-id=none ";

	if (!standard.empty())
		flags += " -std=" + standard + " ";
	
	//automation
	if (all_include_paths.empty())
	{
		if (fs::is_directory("include"))
			all_include_paths = " -Iinclude ";
		else if (fs::is_directory("inc"))
			all_include_paths = " -Iinc ";
	}
}

void Class::smolize()
{
	if(!smol && !force_smol) return;
	Util::system("strip -S --strip-unneeded --remove-section=.note.gnu.gold-version "
			"--remove-section=.comment --remove-section=.note --remove-section=.note.gnu.build-id --remove-section=.note.ABI-tag "
			+ out_name);
}

bool Class::ask_to_install()
{
#ifdef __WIN32
	return false;
#endif // __WIN32
	if(dont_ask_install) return true;
	fflush(stdin); std::flush(std::cout);

	char answer;
	cout << BLUE "Install \"" << name << "\"? " COLOR_RESET "("
		GREEN "Y" YELLOW "\\" RED "n" COLOR_RESET"): ";
	std::cin >> answer;

	return answer == 'Y' || answer == 'y';
}

void Class::create_directories()
{
	Util::create_folder(object_dir.c_str());
	
	string path = out_name.substr(0, out_name.find_last_of('/')+1);
	
	if (!path.empty() && path != "./")
		Util::recursively_create_folder(path.c_str());
}