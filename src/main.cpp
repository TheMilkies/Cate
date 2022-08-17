#include "Parser.hpp" //Parser.hpp includes everything we need, including Util.hoo

//parser_exit is needed to show all errors and exit afterwards
//system_allowed is the -D option, only affects `system(String)` in parser
bool parser_exit = false, system_allowed = true;

int thread_count = std::thread::hardware_concurrency() * 2;

void help()
{
	std::cout << CYAN BOLD "Cate " CATE_VERSION "\n"
	BOLD CYAN "usage:"     "\tcate " GREEN " [FLAGS] " ORANGE "[FILENAME]\n\n" COLOR_RESET
	BOLD CYAN "flags:\n" COLOR_RESET
	BOLD GREEN "\t-t" ORANGE "N" COLOR_RESET ": sets thread count to " ORANGE BOLD "N\n" COLOR_RESET
	BOLD GREEN "\t-D" COLOR_RESET ":  disables all " YELLOW "system()" COLOR_RESET " calls in script\n"
	BOLD GREEN "\t-v" COLOR_RESET ":  shows version\n";
	exit(1);
}

int main(int argc, char *argv[])
{
	std::ios_base::sync_with_stdio(false); //this is a massive speed boost in some cases.

	if (argc < 2) help();

	string file_name; file_name.reserve(64); //the filename, we will get it with the following for loop

	string arg; arg.reserve(64); //current command-line argument in std::string form, it's easier to work with it like this

	//ARGC_START is where argc should start, 0 in windows, 1 in linux and other sane operating systems.
	for (int i = ARGC_START; i < argc; i++)
	{
		arg = argv[i];
		
		if (arg[0]== '-')
		{
			switch (arg[1]) //check the second character of the argument
			{
			case 't':
				if (arg.length() < 3) //if just "-t"
					Util::command_error("Invalid argument " + arg);

				thread_count = std::stoi(arg.substr(2, arg.length()));
				break;

			case 'v': //cate version
				std::cout << CATE_VERSION "\n";
				return 0; //exit after
				break;
			
			case 'D': //disable system
				system_allowed = false;
				break;
			
			default: //unknown
				Util::command_error("Unknown argument " + arg);
				break;
			}
		}
		else //must be a filename now
		{
			file_name = argv[i];
			//add cate file ending if doesn't already end with '"".cate"
			if (!Util::ends_with(arg, ".cate"))
				file_name += ".cate";
		}
	}

	if (file_name.empty()) //incase of no file
		Util::command_error("No input file");

	Parser parser(file_name); //start parsing
	return 0;
}