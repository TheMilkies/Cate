#include "Parser/Parser.hpp"

//add new methods in include/Class.hpp, then implement them here

//add them here first with `else if (child == "THING")`
bool Parser::object_method()
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
		return false; //if not any of those, it's a property
	}

	return true; //will go to the next token
}

void Class::clean() 
{
	if (out_dir.empty())
		out_dir = "build";

	string command =
#ifdef __WIN32
	"del -f -q "	
#else
	"rm -f " 
#endif // OS Check
	+ out_dir + "/*" OBJ_EXTENSION;
	Util::system(command);
}