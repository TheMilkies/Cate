#include "Project.hpp"

Project::Project() {}

Project::~Project() {}

void Project::build() 
{
	//one file
	if(files.size() == 1)
	{
		if(newer_than(out_name, files[0]))
		{
			std::thread include_thread(&Class::include_setup, this);
			std::thread library_thread(&Class::library_setup, this);

			include_thread.join();
			library_thread.join();

			create_directories();

			files[0] += ' ';
			Util::system (
				//command = $CC -o$OUT_NAME $FILE $FLAGS $LIB_PATHS $LIBS $INCS $FINAL_FLAGS
				command_gen(files[0])
			);
			goto done;
		}
		already_built = true;
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
		if (!Util::file_exists(out_name.c_str()))
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


inline string Project::command_gen(string& objects)
{
	return compiler + " -o" + out_name + " " +
		   objects + flags + " " +
		   all_library_paths + all_libraries + " " +
		   all_include_paths + final_flags;
}