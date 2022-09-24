#include "Parser.hpp"

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

#ifdef __WIN32
	string command = "del -f -q " + out_dir + "/*" OBJ_EXTENSION;	
#else
	string command = "rm -f " + out_dir + "/*" OBJ_EXTENSION;
#endif // OS Check
	Util::system(command);
}