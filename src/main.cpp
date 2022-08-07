#include "Parser.hpp"

bool parser_exit;

int thread_count = 2;

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
		std::cout << "Cate v1.0.0\nusage: `cate [FILENAME]`";
		return 1;
	}

	char*file_name;

	for (int i = 1; i < argc; i++)
	{
		string arg = argv[i];
		if (arg.rfind("-t", 0) == 0)//if starts with
		{
			thread_count= std::stoi(arg.substr(2, arg.length()));
		}
		else
		{
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