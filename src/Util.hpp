#if !defined(Util_HPP)
#define Util_HPP
#include "inc.hpp"
#define CATE_VERSION "v1.2.10 (Development)"

#ifdef __WIN32
	#define ARGC_START 0
	#define OBJ_EXTENSION ".obj" //windows, why?
#else
	#define ARGC_START 1
	#define OBJ_EXTENSION ".o"
#endif // OS check

extern int32_t lexer_line;

#define BOLD "\033[1m" 
#define COLOR_RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define PURPLE "\033[35m"
#define CYAN "\033[36m"

namespace Util
{
	void error(string_view problem);

	void command_error(string_view problem);
	void lexer_error(std::string problem); //has the std here because flex /neg
	void error(int32_t line, string_view problem);
	void fatal_error(int32_t line, string_view problem);
	void build_error(string_view name, string_view problem);

	inline string remove_extension(string& s) {return s = s.substr(0, s.find_last_of("."));}
	string remove_quotes(string &s);

	//there are two of these because i'm lazy.
	void replace_all(string& s, string_view toReplace, string_view replaceWith);
	string replace_all_safe(string_view s, string_view toReplace, string_view replaceWith);

	/// @brief Returns the modified time of the given path 
	/// @param path Path to check
	/// @return 0 if file doesn't exist, the modified time if it does.
	long long get_modified_time(const char *path);
	inline bool file_exists(const char* file_name) { return access(file_name, F_OK) != -1; } 

	/// @brief Creates a folder if it doesn't already exist.
	/// @param path Path of the folder to create
	inline void create_folder(const char* path) {
		struct stat st {0};

		if (stat(path, &st) == -1) { //if folder does not exist.
		#ifdef __WIN32
			if (!CreateDirectory(path, NULL))
		#else
			if (mkdir(path, 0700) && errno != EEXIST)
		#endif // __OSCheck
				fatal_error(0, string("Could not create folder \"") + path + "\"");
		}
	} 

	/// @brief Like std::system, but exits if the command returns anything other than 0.
	/// @param command The command to execute.
	void system(string_view command);

	bool ends_with(string_view value, string_view ending); //written by tshepang from stackoverflow
} // namespace Util

#endif