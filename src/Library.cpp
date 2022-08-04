#include "Library.hpp"

Library::Library()
{
}

Library::~Library()
{
}

void Library::build() 
{
	setup();
	build_objects();
	if (!needs_rebuild)
		return;
	
	string command;
	if (is_static)
		command = "ar rcs -o " + out_name + " " + all_object_files + " ";
	else
		command = compiler + " -shared -fpic -o " + out_name + " " + all_object_files + " " + flags;
	
	Util::system(command);
	std::cout << "\x1B[32mDone building \"" << name << "\"\u001b[0m\033[0m\n";
};