#if __has_include(<experimental/filesystem>)
    #include <experimental/filesystem>
  	namespace fs = std::experimental::filesystem;
#elif __has_include(<filesystem>)
	#include <filesystem>
	namespace fs = std::filesystem;
#else
    #error "no filesystem support"
#endif
#include <iostream>

int main(int argc, char const *argv[])
{
	for (const auto &p : fs::directory_iterator(".")) //iterate over the files
	{
		if (p.path().extension() == ".out")
		{
			std::cout << p.path().stem();
		}
	}
	return 0;
}
