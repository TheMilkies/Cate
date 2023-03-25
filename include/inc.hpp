#ifndef INC_HPP
#define INC_HPP
//global includes for easier life

#include <iostream>
#include <vector>

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

using std::string;
using std::string_view;
using std::vector;
using std::cout;
using std::cerr;
typedef int32_t i32;

#endif