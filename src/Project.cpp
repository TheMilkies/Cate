#include "Project.hpp"

Project::Project() {}

Project::~Project() {}

void Project::build() 
{
	//same as library

	if (!already_built)
	{
		setup();
		build_objects();
	}

	if (link)
	{
		if (!Util::file_exists(out_name.c_str())) //if file doesn't exist
		needs_rebuild = true;

		if (!needs_rebuild) return;
	
		string command = compiler + " -o " + out_name + " " + all_object_files + flags + " " + all_library_paths + all_libraries + " " + all_include_paths + final_flags;

		Util::system(command);
	}
	
	std::cout << GREEN "Done building \"" << name << "\"" COLOR_RESET "\n";
}