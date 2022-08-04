#include "Parser.hpp"

bool parser_exit;

int main(int argc, const char *argv[])
{
	std::ios_base::sync_with_stdio(false);

	if (argc < 2)
	{
		std::cout << "Cate v1.0.0\nusage: `cate [FILENAME]`";
		return 1;
	}

	Parser parser(argv[1]);
	parser.parse();

	return 0;
}
