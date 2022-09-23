#include "Parser.hpp" //Parser.hpp includes everything we need, including Util.hpp

//parser_exit is needed to show all errors and exit afterwards
//system_allowed is the -D option, only affects `system(String)` in parser
bool parser_exit = false, system_allowed = true, force_rebuild = false;

int32_t thread_count = std::thread::hardware_concurrency() * 2;

void help()
{
	std::cout << BLUE BOLD "Cate " CATE_VERSION "\n"
	"usage: " COLOR_RESET "\tcate " BOLD GREEN " [FLAGS] " PURPLE "[FILENAME]\n\n" COLOR_RESET
	BOLD GREEN "flags:\n"
	"\t-h" COLOR_RESET ":  shows help (this)" BOLD GREEN "\n"
	"\t-t" YELLOW "N" COLOR_RESET ": sets thread count to " YELLOW BOLD "N\n"
	GREEN "\t-D" COLOR_RESET ":  disables all " YELLOW "system()" COLOR_RESET " calls in script\n"
	BOLD GREEN "\t-v" COLOR_RESET ":  shows version\n"
	BOLD GREEN "\t-f" COLOR_RESET ":  delete everything in class's " YELLOW "build_directory" COLOR_RESET "; force rebuild\n";
}

string file_name;

bool parse_catel();

int main(int argc, char *argv[])
{
	std::ios_base::sync_with_stdio(false); //this is a massive speed boost in some cases.
	bool catel = Util::file_exists(".catel"),
		 default_file = Util::file_exists("build.cate"),
		 default_file_in_dir = Util::file_exists("cate/build.cate");

	if (argc < 2 && !catel && !default_file && !default_file_in_dir)
	{
		help();
		return 1;
	}

	file_name.reserve(64); //the filename, we will get it with the following for loop

	//ARGC_START is where argc should start, 0 in windows, 1 in linux and other sane operating systems.
	for (int32_t i = ARGC_START; i < argc; i++)
	{		
		if (argv[i][0] == '-')
		{
			switch (argv[i][1]) //check the second character of the argument
			{
			case 't': {
				if (argv[i][2] == NULL) //if just "-t"
					Util::command_error("Missing argument \"-t\"");

				int sub = atoi((char*)argv[i] + 2); //get everything after "-t"
				if (sub != 0) //if 0 or invalid
					thread_count = sub;
				
			}	break;

			case 'v': //cate version
				std::cout << CATE_VERSION "\n";
				return 0; //exit after
				break;

			case 'h': //cate help
				help();
				return 0; //exit after
				break;
			
			case 'D': //disable system
				system_allowed = false;
				break;

			case 'f': //force rebuild
				force_rebuild = true;
				break;
			
			default: //unknown
				//i'm lazy here
				std::cout << RED BOLD "Error" COLOR_RESET " in command: Unknown argument: \""
						  << argv[i] << "\"\n";
				return 1;
				break;
			}
		}
		else //must be a filename now
		{
			if (file_name.empty())
				file_name = argv[i];
		}
	}

	if (catel)
		parse_catel();
	
	if (file_name.empty() || file_name == ".cate") //incase of no file
	{
		if (default_file)
		{
			file_name = "build.cate";
			goto skip_check;
		}
		else if(default_file_in_dir)
		{
			file_name = "cate/build.cate";
			goto skip_check; //fastest and cleanest way
		}
		else
		{
			if (fs::is_directory("cate") && !catel)
				Util::command_error("No input file, maybe create a .catel file?");

			Util::command_error("No input file");
		}
	}

	//add cate file ending if doesn't end with ".cate"
	if (!Util::ends_with(file_name, ".cate"))
		file_name += ".cate";

skip_check:
	Parser parser(file_name); //start parsing

	return 0;
}

//this is so bad but it works so well
bool parse_catel()
{
	std::ifstream file(".catel");

	if (file.fail())
		return false;

	string s1, s2, dir = "cate", def = "build.cate";

	while (file >> s1 >> s2)
	{
		if (s1.empty() || s2.empty())
			Util::fatal_error(0, "Catel file error. one feild is empty");
		
		if (s1 == "dir" || s1 == "directory")
			dir = s2;
		else if (s1 == "def" || s1 == "default")
			def = s2;
	}

	//do file_name stuff
	if (!def.empty() && file_name.empty())
		file_name = def;

	if (dir.empty())
		return false;

	if (!Util::ends_with(file_name, ".cate"))
		file_name += ".cate";

	string file_name_with_dir = dir + "/" + file_name;
	
	if (Util::file_exists(file_name_with_dir.c_str()))
	{
		file_name = file_name_with_dir;
	}
	else
		return false; //will continue as normal cate file

	//done
	return true;
}