#include "Project.hpp"

Project::Project() {}

Project::~Project() {}

void Project::build() 
{
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
			compiler + " -o" + out_name + " " +
			all_object_files + flags + " " +
			all_library_paths + all_libraries + " " +
			all_include_paths + final_flags
		);
	}
	
	smolize();
	std::cout << GREEN "Done building \"" << name << "\"" COLOR_RESET "\n";
}