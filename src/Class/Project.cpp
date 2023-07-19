#include "Class/Project.hpp"

Project::Project(string_view ident): Class(ident) {}
Project::~Project() {}

void Project::generate_name()
{
	out_name = name
#ifdef __WIN32
	+= ".exe"
#endif
	;
}

void Project::set_type(i32 line, bool is_static)
{
	Util::warn("Projects don't have the " hl_var("type") " property.", line);
}

void Project::build()
{
	//one file
	if(files.size() == 1) return single_file_build();

	//multi file
	if (!already_built)
	{
		setup();
		build_objects();
	}

	if (link)
	{
		if (!needs_link) return;

		Util::system(
			//command = $CC -o$OUT_NAME $OBJ $FLAGS $LIB_PATHS $LIBS $INCS $FINAL_FLAGS
			generate_command_for(all_object_files)
		);
	}

	smolize();
	print_done_message_with(name);
}

void Project::single_file_build()
{
	if(!already_built) check();

	string& file = files[0];
	if(newer_than(out_name, file) || flag(force_rebuild))
	{
		create_directories();

		file += ' '; //needed because it'd be $FILE$FLAGS (compiler doesn't like that.)

		Util::system (
			//command = $CC -o$OUT_NAME $FILE $FLAGS $LIB_PATHS $LIBS $INCS $FINAL_FLAGS
			generate_command_for(file)
		);

		already_built = true;
		smolize();
		print_done_message_with(name);
	}
}

inline string Project::generate_command_for(string& objects)
{
	return compiler + " -o" + out_name + " " +
		   objects + flags + " " + all_definitions +
		   all_libraries + " " +
		   all_include_paths + final_flags;
}