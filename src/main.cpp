#include "Parser.hpp"

bool parser_exit;

int main(int argc, const char *argv[])
{
	if (argc < 1)
	{
		std::cout << "Cate " CATE_VERSION "\n"
								 "usage: `cate [FILENAME]`";
		return 1;
	}

	const char* file = argv[1];

	std::ios_base::sync_with_stdio(false);
	
	Parser parser(file);
	parser.parse();
	return 0;
}
