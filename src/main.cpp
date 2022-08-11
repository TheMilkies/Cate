#include "Parser.hpp"

bool parser_exit, system_allowed = true;

int thread_count = std::thread::hardware_concurrency() * 2;

inline bool ends_with(std::string const & value, std::string const & ending) //written by tshepang from stackoverflow
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

void help()
{
	std::cout << "\x1b[34mCate " CATE_VERSION "\x1b[0m\n"
	BOLD "usage:" RESET "\tcate [FLAGS] [FILENAME]\n\n"
	BOLD "flags:\n" RESET
	"\t-tN: sets thread count to N\n"
	"\t-v:  shows version\n"
	"\t-D:  disable all script defined `system()` calls";
	exit(1);
}

int main(int argc, char *argv[])
{
	std::ios_base::sync_with_stdio(false); // because SPEED

	if (argc < 2)
		help();

	char*file_name;

	for (int i = ARGC_START; i < argc; i++)
	{
		string arg = argv[i];
		
		if (arg[0]== '-')
		{
			switch (arg[1])
			{
			case 't':
				if (arg.length() < 3) //if just "-t"
					Util::command_error("Invalid argument " + arg);
				else
					thread_count = std::stoi(arg.substr(2, arg.length()));
				break;

			case 'v':
				std::cout << CATE_VERSION "\n";
				return 0;
				break;
			
			case 'D':
				system_allowed = false;
				break;
			
			default:
				Util::command_error("Unknown argument " + arg);
				break;
			}

		}
		else
		{
			if (arg == "cate") continue; //why is this bug a thing?
			
			if (ends_with(arg, ".cate"))
				file_name = (char*)argv[i];
			else //add .cate ending
				file_name = strcat(argv[i], ".cate");
		}
	}

	if (file_name == NULL)
		Util::command_error("No input file");

	Parser parser(file_name);

	return 0;
}