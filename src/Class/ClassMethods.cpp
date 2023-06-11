#include "Parser/Parser.hpp"
#include <string.h>

//add new methods in include/Class.hpp, then implement them here

//add them here first with `else if (child == "THING")`
void Parser::object_method()
{
	void_function(); 
	if (child == "build") //void Class.build(void);
		current_class->build();	//in Project.cpp/Library.cpp			
	else if (child == "install") 
		current_class->install();				
	else if (child == "clean") //void Class.build(void);
		current_class->clean();				
	else
	{
		fatal("\"" YELLOW + child + "()" COLOR_RESET "\" is not a valid member function\n"
	 	"Avilable are:\n"
		"  * " hl_func("build()") 	"\n"
		"  * " hl_func("install()") "\n"
		"  * " hl_func("clean()")
		);
	}
}

void Class::clean() 
{
	if (object_dir.empty())
	{
		if(global_values.object_dir.empty())
			Util::generate_object_dir_name();
		object_dir = global_values.object_dir;
	}

	if(object_files.empty())
		setup_objects();
	
	for(auto& obj : object_files)
	{
		if(remove(obj.c_str()) != 0 && errno != ENOENT)
			Util::error("Can not delete \"" + obj + "\" because: " + strerror(errno) +
			"Suggestion: try running `"
			SUDO "rm " COLOR_RESET
			PURPLE + obj + COLOR_RESET);
	}

	object_files.clear();
	all_object_files.clear();
	already_built = false;
}