#include "Util.hpp"

extern bool parser_exit;

namespace Util
{
	void error(string_view problem)
	{
		std::cout << RED BOLD "Error" COLOR_RESET ": " << problem << "\n";
		parser_exit = true;
	}

	void lexer_error(string problem)
	{
		std::cout << RED BOLD "Error" COLOR_RESET " in line " << lexer_line << ": " << problem << "\n";
		parser_exit = true;
	}

	void error(int32_t line, string_view problem)
	{
		std::cout << RED BOLD "Error" COLOR_RESET " in line " << line << ": " << problem << "\n";
		parser_exit = true;
	}

	void fatal_error(int32_t line, string_view problem)
	{
		std::cout << RED BOLD "Error" COLOR_RESET " in line " << line << ": " << problem << "\nTerminating.\n";
		exit(1);
	}

	void command_error(string_view problem)
	{
		std::cout << RED BOLD "Error" COLOR_RESET " in command: " << problem << "\n";
		exit(1);
	}

	void build_error(string_view name, string_view problem)
	{
		std::cout <<  RED BOLD "Error" COLOR_RESET ": Cannot build \""
						  << name << "\" because " << problem << "\nTerminating.\n";
		exit(1);
	}

	string remove_quotes(string &s) 
	{
		s.erase(0, 1); //remove first char
		s.pop_back(); //remove last char
		return s;
	}

	void replace_all( //thank you for the code @Mateen Ulhaq from stackoverflow! i was too lazy to write it myself
		string& s,
		string_view toReplace,
		string_view replaceWith
	) {
		std::ostringstream oss;
		oss.str().reserve(s.length());
		std::size_t pos = 0;
		std::size_t prevPos = pos;

		while (true) {
			prevPos = pos;
			pos = s.find(toReplace, pos);
			if (pos == string::npos)
				break;
			oss << s.substr(prevPos, pos - prevPos);
			oss << replaceWith;
			pos += toReplace.size();
		}

		oss << s.substr(prevPos);
		s = oss.str();
	}

	/*string replace_all_safe( //thank you for the code @Mateen Ulhaq from stackoverflow! i was too lazy to write it myself
		string_view s,
		string_view toReplace,
		string_view replaceWith
	) {
		std::ostringstream oss;
		oss.str().reserve(s.length());
		std::size_t pos = 0;
		std::size_t prevPos = pos;

		while (true) {
			prevPos = pos;
			pos = s.find(toReplace, pos);
			if (pos == string::npos)
				break;
			oss << s.substr(prevPos, pos - prevPos);
			oss << replaceWith;
			pos += toReplace.size();
		}

		oss << s.substr(prevPos);
		return oss.str();
	}*/

	long long get_modified_time(const char *path)
	{
		struct stat attr;
		if (stat(path, &attr) != 0) //check if file exists
			return 0; //will always recompile since file doesn't exist

		//return last modified time
	#ifdef __WIN32
		return attr.st_mtime;
	#else
		return attr.st_mtim.tv_sec; //posix why must you have the worse syntax for this?
	#endif // OS check tm
	}

	//no system_allowed check here because it's ran by build threads
	void system(string_view command)
	{
		int32_t ret = std::system(command.data());

#ifdef __WIN32
		if (((ret) & 0x7f) && (((ret) & 0xff00) >> 8) != 0)
#else
		if (WIFEXITED(ret) && WEXITSTATUS(ret) != 0)
#endif // __WIN32
		{
			std::cout << RED BOLD "Error" COLOR_RESET ": Error in command/build command.\n";
			exit(1);
		}
	}

	bool ends_with(string_view value, string_view ending) //written by tshepang from stackoverflow, should be rewritten
	{
		if (ending.size() > value.size()) return false;
		return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
	}
} // namespace Util
