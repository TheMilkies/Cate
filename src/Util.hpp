#if !defined(Util_HPP)
#define Util_HPP
#include "inc.hpp"
#define CATE_VERSION "v1.0.0"

extern bool lexer_exit;
extern int lexer_line;

namespace Util
{
	void error(string problem);
	void lexer_error(std::string problem);
	void error(int line, string problem);
	void build_error(string name, string problem);
	string remove_quotes(string &s);
	void replace_all(string& s, string const& toReplace, string const& replaceWith);
	string replace_all_safe(string s, string const& toReplace, string const& replaceWith);
	string remove_extention(string& s);
	long get_modified_time(const char *path);
	void system(string &command);
} // namespace Util


#endif
