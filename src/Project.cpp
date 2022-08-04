#include "Project.hpp"

Project::Project()
{
}

Project::~Project()
{
}

void Project::build() 
{
	setup();
	build_objects();
	if (!needs_rebuild)
		return;
	string command = compiler + " -o " + out_name + " " + all_object_files + " " + flags + " " + all_library_paths + ' ' + all_libraries;
	std::cout << command << "\n";
	Util::system(command);
	std::cout << "\x1B[32mDone building \"" << name << "\"\u001b[0m\033[0m\n";
};