#if !defined(Util_HPP)
#define Util_HPP
#include "inc.hpp"
#define CATE_VERSION "v1.1.2"

#ifdef __WIN32
#define ARGC_START 0
#else
#define ARGC_START 1
#endif // OS check

extern bool lexer_exit;
extern int lexer_line;

namespace Util
{
	void error(string problem);
	void lexer_error(std::string problem);
	void error(int line, string problem);
	void fatal_error(int line, string problem);
	void build_error(string name, string problem);

	inline string remove_extention(string& s) {return s = s.substr(0, s.find_last_of("."));}
	string remove_quotes(string &s);

	void replace_all(string& s, string const& toReplace, string const& replaceWith);
	string replace_all_safe(string &s, string const& toReplace, string const& replaceWith);

	long get_modified_time(const char *path);
	void system(string &command);
} // namespace Util


#endif
