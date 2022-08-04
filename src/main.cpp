#include "Parser.hpp"

bool parser_exit;
int total_alloc = 0;

void* operator new(size_t size)
{
	total_alloc++;
	return malloc(size);
}

int main(int argc, char const *argv[])
{
	std::ios_base::sync_with_stdio(false);
	Parser parser(argv[1]);
	using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;

    auto t1 = high_resolution_clock::now();
	parser.parse();
    auto t2 = high_resolution_clock::now();

	auto ms_int = duration_cast<milliseconds>(t2 - t1);
	std::cout << ms_int.count() << "ms\n";
	return 0;
}
