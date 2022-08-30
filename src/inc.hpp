#ifndef INC_HPP
#define INC_HPP
//global includes for easier life

#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <sstream>

#ifdef __WIN32
	#include<windows.h>
#endif // __WIN32

#if __has_include(<experimental/filesystem>)
    #include <experimental/filesystem>
  	namespace fs = std::experimental::filesystem;
#elif __has_include(<filesystem>)
	#include <filesystem>
	namespace fs = std::filesystem;
#else
    #error "no filesystem support"
#endif

#if __has_include(<string_view>) && __cplusplus >= 201703L
  #include <string_view>
#else
  #include "nonstd/string_view.hpp"
#endif

#if __has_include(<thread>)
	#ifdef __WIN32
		#include "nonstd/mingw_threads.hpp"
	#else
		#include <thread>
	#endif // OS Check
#else
    #error "no threading support"
#endif

#include <chrono>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unordered_map>
#include "robin_hood.hpp"

using std::string;
using std::string_view;
using std::vector;
using std::unordered_map;
#endif