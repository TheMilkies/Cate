bool call_competitor()
{
	std::ios_base::sync_with_stdio(true);
	using namespace Util;
	if(file_exists("Makefile") || file_exists("makefile"))
	{
		cout << "No catefiles found, but found a makefile.\nRunning Make\n";
		system("make -j" + std::to_string(thread_count));
		return true;
	}
	else if(file_exists("CMakeLists.txt"))
	{
		cout << "No catefiles found, but found CMake.\nRunning CMake\n";
		system("mkdir build;cd build;make -j" + std::to_string(thread_count));
		return true;
	}
	return false;
}