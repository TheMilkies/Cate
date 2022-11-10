#include "Util.hpp"
#include "inc.hpp"

extern string file_name, dir;

//this is so bad but it works so well
void parse_catel()
{
	using namespace Util;

	std::ifstream file(".catel");
	if (file.fail()) return;

	string s1, s2, def = "build.cate";

	while (file >> s1 >> s2)
	{
		if (s1.empty() || s2.empty())
			fatal_error(0, "Catel file error. one feild is empty");
		
		if (s1 == "dir" || s1 == "directory")
			dir = s2;
		else if (s1 == "def" || s1 == "default")
			def = s2;
	}

	//do file_name stuff
	if (!def.empty() && file_name.empty())
		file_name = def;

	if (dir.empty() || file_name.empty()) return;

	if (!ends_with(file_name, ".cate"))
		file_name += ".cate";

	string file_name_with_dir = dir + "/" + file_name;
	
	if (file_exists(file_name_with_dir.c_str()))
		file_name = file_name_with_dir;

	//done
	return;
}