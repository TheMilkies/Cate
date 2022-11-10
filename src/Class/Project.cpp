#include "Class/Project.hpp"

Project::Project() {}

Project::~Project() {}

void Project::generate_name()
{
	out_name = name
#ifdef __WIN32
	+= ".exe"
#endif
	;
}

void Project::build() 
{
	//one file
	if(files.size() == 1)
	{
		single_file_build();
		goto done;
	}

	//multi file
	if (!already_built)
	{
		setup();
		build_objects();
	}

	if (link)
	{
		//if file doesn't exist
		if (!Util::file_exists(out_name.c_str())
		||   fs::is_directory(out_name.c_str()))
			needs_rebuild = true;

		if (!needs_rebuild) return;

		Util::system(
			//command = $CC -o$OUT_NAME $OBJ $FLAGS $LIB_PATHS $LIBS $INCS $FINAL_FLAGS
			command_gen(all_object_files)
		);
	}
done:
	smolize();
	print_done(name);
}

void Project::single_file_build()
{
	string& file = files[0];
	if(newer_than(out_name, file))
	{
		if(!already_built) check();

		create_directories();

		file += ' ';

		Util::system (
			//command = $CC -o$OUT_NAME $FILE $FLAGS $LIB_PATHS $LIBS $INCS $FINAL_FLAGS
			command_gen(file)
		);

		already_built = true;
	}
}

inline string Project::command_gen(string& objects)
{
	return compiler + " -o" + out_name + " " +
		   objects + flags + " " +
		   all_library_paths + all_libraries + " " +
		   all_include_paths + final_flags;
}