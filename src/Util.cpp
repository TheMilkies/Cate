#include "Util.hpp"

extern bool parser_exit;

#ifdef __WIN32
	#include<windows.h>
#endif // __WIN32

#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#ifdef __WIN32
	#define WIFEXITED(status) ((status) & 0x7f)
	#define WEXITSTATUS(status) (((status) & 0xff00) >> 8)
#endif // __WIN32


using std::cout;

namespace Util
{
	void error(string_view problem)
	{
		cout << ERROR ": " << problem << "\n";
		parser_exit = true;
	}

	void lexer_error(string problem)
	{
		cout << ERROR " in line " << lexer_line << ": " << problem << "\n";
		parser_exit = true;
	}

	void error(int32_t line, string_view problem)
	{
		cout << ERROR " in line " << line << ": " << problem << "\n";
		parser_exit = true;
	}

	void fatal_error(int32_t line, string_view problem)
	{
		cout << ERROR " in line " << line << ": " << problem << "\nTerminating.\n";
		exit(1);
	}

	void command_error(string_view problem)
	{
		cout << ERROR " in command: " << problem << "\n";
		exit(1);
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

	long long get_modified_time(const char *path)
	{
		struct stat attr;
		if (stat(path, &attr) != 0) //check if file exists
			return 0; //will always recompile since object file doesn't exist

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
		if (command.empty()) return;

		int32_t ret = std::system(command.data());

		if (WIFEXITED(ret) && WEXITSTATUS(ret) != 0)
		{
			cout << ERROR ": Error in build command.\n";
			exit(1);
		}
	}

	void user_system(int32_t line, string_view command)
	{
		if (command.empty()) return;
	
		int32_t ret = std::system(command.data());
		int32_t exit_status = WEXITSTATUS(ret);

		if (WIFEXITED(ret) && exit_status != 0)
		{
			cout << ERROR " in " hl_func("system") " call ran by line " << line << ".\n"
			"Ran \"" << command << "\"\nExited with code " << exit_status << '\n';
		}

		cout << command << "\n";
	}

	bool ends_with(string_view value, string_view ending) //written by tshepang from stackoverflow, should be rewritten
	{
		if (ending.size() > value.size()) return false;
		return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
	}

	void recursively_create_folder(const char* path) //written by Neuron from stackoverflow
	{
		struct stat st {0};
		if (stat(path, &st) != -1) return;

		char tmp[512];
		char *p = NULL;
		size_t len;

		snprintf(tmp, sizeof(tmp),"%s",path);
		len = strlen(tmp);
		if (tmp[len - 1] == '/')
			tmp[len - 1] = 0;
		for (p = tmp + 1; *p; p++)
			if (*p == '/') {
				*p = 0;
				create_folder(tmp);
				*p = '/';
			}
		create_folder(tmp);
	}

	void create_folder(const char* path)
	{
		struct stat st {0};
		if (stat(path, &st) != -1) return; //if folder does not exist.
		
	#ifdef __WIN32
		if (!CreateDirectory(path, NULL))
	#else
		if (mkdir(path, 0700) && errno != EEXIST)
	#endif // OSCheck2
			fatal_error(0, string("Could not create folder \"") + path + "\"\n"
			"Maybe try running \"" BLUE
			"mkdir "
#ifndef __WIN32
			GREEN "-p "
#endif // unix
			PURPLE + path + COLOR_RESET "\""
			);
	}

	void add_cate_ending(std::string& s)
	{
		if (!s.empty() && !ends_with(s, ".cate"))
			s += ".cate";
	}

	//coming maybe soon
	/*void check_root()
	{
		if (getuid() != 0)
		{
			Util::error("Catefile wants to install but user is not root.");
			exit(2);
		}
	}*/
} // namespace Util
