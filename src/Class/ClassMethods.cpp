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
		fatal(string("Unknown member function \"" YELLOW) +
		child.c_str() + "()" COLOR_RESET "\"");
	}
}

void Class::clean() 
{
	if (object_dir.empty())
		object_dir = "build";

	string command =
#ifdef __WIN32
	"del -f -q "	
#else
	"rm -f " 
#endif // OS Check
	+ object_dir + "/*" OBJ_EXTENSION;
	Util::system(command);
}