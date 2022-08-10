#include "Project.hpp"

Project::Project()
{
}

Project::~Project()
{
}

void Project::build() 
{
	if (!already_built)
	{
		setup();
		build_objects();
	}
	
	if (Util::get_modified_time(out_name.c_str()) == 0) //if file doesn't exist
		needs_rebuild = true;

	if (!needs_rebuild)
		return;

	string command = compiler + " -o " + out_name + " " + all_object_files + flags + " " + all_library_paths + all_libraries + " " + all_include_paths + final_flags;
	Util::system(command);
	std::cout << "\x1B[32mDone building \"" << name << "\"\u001b[0m\033[0m\n";
};