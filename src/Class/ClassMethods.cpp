#include "Parser/Parser.hpp"

//add new methods in include/Class.hpp, then implement them here

//add them here first with `else if (child == "THING")`
void Parser::object_method()
{
	if (child == "build") //void Class.build(void);
	{
		void_function(); 
		current_class->build();	//in Project.cpp/Library.cpp			
	}
	else if (child == "clean") //void Class.build(void);
	{
		void_function(); 
		current_class->clean();				
	}
	else
	{
		fatal(string("\"" YELLOW) +
		child.c_str() + "()" COLOR_RESET "\" is not a valid member function");
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

	string command =
#ifdef __WIN32
	"del -f -q "	
#else
	"rm -f " 
#endif // OS Check
	+ object_dir + "/*" OBJ_EXTENSION;
	Util::system(command);
}