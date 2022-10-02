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
	"\t-l" COLOR_RESET ":  list all cate files in default directory\n"
	BOLD GREEN "\t-t" highlight_var("N") ": sets thread count to " PURPLE BOLD "N\n"
	GREEN "\t-D" COLOR_RESET ":  disables all " highlight_func("system()") " calls in script\n"
	BOLD GREEN "\t-f" COLOR_RESET ":  delete everything in class's " highlight_var("build_directory") "; force rebuild\n"
	BOLD GREEN "\t-v" COLOR_RESET ":  shows version\n"
	BOLD GREEN "\t-h" COLOR_RESET ":  shows help (this)" BOLD GREEN "\n";
}

string file_name, dir = (fs::is_directory("cate") == true) ? "cate" : "./";

void parse_catel(); // Catel.cpp

int main(int argc, char *argv[])
{
	std::ios_base::sync_with_stdio(false); //this is a massive speed boost in some cases.
	using namespace Util;

	file_name.reserve(64);

	bool catel_exists = file_exists(".catel");

	//get default file
	if (argc < 2 && !catel_exists)
	{
		if (file_exists("build.cate"))
		{
			file_name = "build.cate";
		}
		else if(file_exists("cate/build.cate"))
		{
			file_name = "cate/build.cate";
		}
		else
		{
			help();
			return 1;
		}
	}
	//get specified
	else
	{
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

				case 'l':{ //list directory
					bool catefiles = false;

					if(catel_exists) parse_catel();

					std::cout << CYAN;
					for (auto &p : fs::directory_iterator(dir)) //iterate over the files
					{
						if (p.path().extension() == ".cate")
						{
							std::cout << p.path().stem().string() << ", ";
							catefiles = true;
						}
					}
					if (catefiles)
						std::cout << COLOR_RESET "\n";
					else
						std::cout << BOLD RED "No catefiles found" COLOR_RESET "\n";

					return 0;
				} break;
				
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
					command_error(
						string("Unknown argument \"") + argv[i] + "\""
					);
					break;
				}
			}
			else //must be a filename now
			{
				if (file_name.empty())
					file_name = argv[i];
				else
					command_error("Cannot build more than one catefile per command.");
			}
		}
	}

	if(catel_exists) parse_catel();

	if(file_name.empty())
		Util::command_error("No input file");

	Parser* parser = new Parser(file_name);

	delete parser; //yay
	return 0;
}