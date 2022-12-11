#include "Class/Library.hpp"

Library::Library() {}

Library::~Library() {}

void Library::generate_name()
{
	out_name = "out/lib" + name;
	out_name += (is_static) ? ".a" : DYNAMIC_EXTENSION;
}

void Library::build() 
{	
	if (!already_built)
	{
		if(!string_find(flags, "-fPIC"))
			flags += " -fPIC "; //make library basically
		setup();
		build_objects();
	}

	const char* build_type = (is_static) ? " (static)" : " (dynamic)";

	if(link)
	{
		if (!needs_rebuild) return;
		
		string command;
		command.reserve(512);
		if (is_static)
			command = "ar rcs -o " + out_name + " " + all_object_files;
		else
			command = compiler + " -g -shared -o " + out_name + " " + all_object_files + final_flags + " " + flags;
		
		Util::system(command);
	}
	
	smolize();
	print_done_message_with(name + build_type);
}