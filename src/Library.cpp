#include "Library.hpp"

Library::Library()
{
}

Library::~Library()
{
}

void Library::build() 
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
	
	string command;
	command.reserve(512);
	if (is_static)
		command = "ar rcs -o " + out_name + " " + all_object_files + " ";
	else
		command = compiler + " -shared -fpic -o " + out_name + " " + all_object_files + final_flags + " " + flags;
	
	Util::system(command);
	std::cout << "\x1B[32mDone building \"" << name << "\"\u001b[0m\033[0m\n";
};