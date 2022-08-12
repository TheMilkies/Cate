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

	if (!Util::file_exists(out_name.c_str())) //if file doesn't exist
		needs_rebuild = true;

	if (!needs_rebuild)
		return;
	
	string command;
	command.reserve(512);
	if (is_static)
	{
		command = "ar rcs -o " + out_name + " " + all_object_files + " ";
		build_type = " (static)";
	}
	else
	{
		command = compiler + " -shared -fpic -o " + out_name + " " + all_object_files + final_flags + " " + flags;
		build_type = " (dynamic)";
	}
	
	Util::system(command);
	std::cout << "\x1B[32mDone building \"" << name + build_type << "\"\u001b[0m\033[0m\n";
}