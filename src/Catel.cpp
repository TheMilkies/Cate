#include "Util.hpp"
#include <fstream>

extern string default_file, default_directory;

//this is so bad but it works so well
void parse_catel()
{
	using namespace Util;
	const char* catel_to_use = ".catel";
	if(file_exists(PLATFORM_CATEL))
		catel_to_use = PLATFORM_CATEL;

	std::ifstream file(catel_to_use);
	if (file.fail()) return;

	string s1, s2, def = "build.cate";

	while (file >> s1 >> s2)
	{
		if (s1.empty() || s2.empty())
			error("Catel file error. one feild is empty");
		
		if (s1 == "def" || s1 == "default")
			default_file = s2;
		else if (s1 == "dir" || s1 == "directory")
			default_directory = s2;
		else
			error("Unknown command in catel \"" + s1 + "\"");
	}

	//do file_name stuff
	if (!def.empty() && default_file.empty())
		default_file = def;

	Util::add_cate_ending(default_file);

	string file_name_with_dir = default_directory + "/" + default_file;
	
	if (file_exists(file_name_with_dir.c_str()))
		default_file = file_name_with_dir;
}