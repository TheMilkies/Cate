#include "Class/Library.hpp"

Library::Library() {}

Library::~Library() {}

void Library::generate_name()
{
	out_name = "out/lib" + name;
	out_name += (is_static) ? ".a" : DYNAMIC_EXTENSION;
}

void Library::set_type(int32_t line, bool is_static)
{
	if(this->is_static == is_static) return;
	this->is_static = is_static;

	if(out_name.empty()) return;

	if(is_static)
	{
		Util::remove_extension(out_name);
		out_name += ".a";
	}
	else
	{
		Util::remove_extension(out_name);
		out_name += DYNAMIC_EXTENSION;
	}

	needs_link += !Util::file_exists(out_name.c_str());
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
		if (!needs_link) return;
		
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