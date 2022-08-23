#if !defined(Util_HPP)
#define Util_HPP
#include "inc.hpp"
#define CATE_VERSION "v1.2.7 (Development)"

#ifdef __WIN32
#define ARGC_START 0
#define OBJ_EXTENSION ".obj"
#else
#define ARGC_START 1
#define OBJ_EXTENSION ".o"
#endif // OS check

extern int lexer_line;

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
	void error(string problem);

	void command_error(string_view problem);
	void lexer_error(std::string problem); //has the std here because flex /neg
	void error(int line, string_view problem);
	void fatal_error(int line, string problem);
	void build_error(string_view name, string_view problem);

	inline string remove_extension(string& s) {return s = s.substr(0, s.find_last_of("."));}
	string remove_quotes(string &s);

	void replace_all(string& s, string_view toReplace, string_view replaceWith);
	string replace_all_safe(string_view s, string_view toReplace, string_view replaceWith);

	long long get_modified_time(const char *path);
	inline bool file_exists(const char* file_name) { return access(file_name, F_OK) != -1; } 
	void system(string_view command);

	bool ends_with(string_view value, string_view ending); //written by tshepang from stackoverflow
} // namespace Util

#endif