#include "Parser.hpp"

bool parser_exit;

int thread_count = std::thread::hardware_concurrency() * 2;

inline bool ends_with(std::string const & value, std::string const & ending) //written by tshepang from stackoverflow
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

int main(int argc, char *argv[])
{
	std::ios_base::sync_with_stdio(false); // because SPEED

	if (argc < 2)
	{
		std::cout << "\x1b[34mCate " CATE_VERSION "\x1b[0m\nusage: cate [FILENAME]\nflags: -tNUMBER: sets thread count to NUMBER`";
		return 1;
	}

	char*file_name;

	for (int i = ARGC_START; i < argc; i++)
	{
		string arg = argv[i];
		
		if (arg[0]== '-')
		{
			if (arg[1] == 't')//if starts with
			{
				if (arg.length() < 3) //if just "-t"
					Util::fatal_error(0, "Invalid argument " + arg);
				else
					thread_count = std::stoi(arg.substr(2, arg.length()));
			}
		}
		else
		{
			if (arg == "cate")
				continue;
			
			if (ends_with(arg, ".cate"))
				file_name = (char*)argv[i];
			else //add .cate ending
				file_name = strcat(argv[i], ".cate");
		}
	}

	Parser parser(file_name);
	parser.parse();

	return 0;
}