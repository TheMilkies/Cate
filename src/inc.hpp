#ifndef INC_HPP
#define INC_HPP
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>

#if __has_include(<filesystem>)
	#include <filesystem>
	namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
    #include <experimental/filesystem>
  namespace fs = std::experimental::filesystem;
#else
    #error "no filesystem support"
#endif

#ifdef __WIN32
#include "windows/minigw_threads.hpp"
#else
#include <thread>
#endif // __WIN32

#include <chrono>
#include <sys/types.h>
#include <sys/stat.h>
#include <unordered_map>
#include "robin_hood.hpp"

using std::string;
using std::vector;
using std::unordered_map;
#endif
