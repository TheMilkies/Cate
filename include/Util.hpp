#if !defined(Util_HPP)
#define Util_HPP
#include "inc.hpp"
#define CATE_VERSION "v2.7 (Development)"
#ifdef MODS
#define CATE_VERSION "v2.6 (Development Modded)"
#endif // MODS

#ifdef __WIN32
	#define ARGC_START 0
	#define OBJ_EXTENSION ".obj" //windows, why?
	#define DYNAMIC_EXTENSION ".dll"
#else
	#define ARGC_START 1
	#define OBJ_EXTENSION ".o"
	#define DYNAMIC_EXTENSION ".so"
#endif // OS check

extern int32_t lexer_line;

#include "colors.hpp"

#include <unistd.h>

namespace Util
{
	void error(string_view problem);
	void error(int32_t line, string_view problem);
	void command_error(string_view problem);
	void lexer_error(std::string problem); //has the std here because flex /neg
	void fatal_error(int32_t line, string_view problem);
	void build_error(string_view name, string_view problem);

	inline string remove_extension(string& s) {return s = s.substr(0, s.find_last_of("."));}

	//there are two of these because i'm lazy.
	void replace_all(string& s, string_view toReplace, string_view replaceWith);
	//string replace_all_safe(string_view s, string_view toReplace, string_view replaceWith);

	/// @brief Returns the modified time of the given path 
	/// @param path Path to check
	/// @return 0 if file doesn't exist, the modified time if it does.
	long long get_modified_time(const char *path);
	inline bool file_exists(const char* file_name) { return access(file_name, F_OK) != -1; } 

	/// @brief Creates a folder if it doesn't already exist.
	/// @param path Path of the folder to create
	void create_folder(const char* path);
	void recursively_create_folder(const char* path); //written by Neuron from stackoverflow

	/// @brief Like std::system, but exits if the command returns anything other than 0.
	/// @param command The command to execute.
	void system(string_view command);
	void user_system(int32_t line, string_view command);

	bool ends_with(string_view value, string_view ending); //written by tshepang from stackoverflow

	void check_root();

	void add_cate_ending(std::string& s);
} // namespace Util

#define hl_func(x) YELLOW x COLOR_RESET
#define hl_var(x) PURPLE x COLOR_RESET
#define hl_flag(x) "\e[1;32m" "\t" x ": " COLOR_RESET

/// @brief Returns if a file is a newer than another 
/// @param a Path to check
/// @param b Other path to check
#define newer_than(a, b) (Util::get_modified_time(a.c_str()) < Util::get_modified_time(b.c_str()))
#define string_find(x, text) (x.find(text) != string::npos)

#endif