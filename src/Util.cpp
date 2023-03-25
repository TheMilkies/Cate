#include "Util.hpp"
#include "Class/Global.hpp"

extern bool errors_exist;

#ifdef __WIN32
	#include <windows.h>
	#define WIFEXITED(status) ((status) & 0x7f)
	#define WEXITSTATUS(status) (((status) & 0xff00) >> 8)
#endif // __WIN32

#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <sstream>

namespace Util
{
	#define ERROR "\e[1;31mError\e[0m" //redefinition because windows sucks
	void error(string_view problem)
	{
		cerr << ERROR ": " << problem << "\n";
		errors_exist = true;
	}

	void error(i32 line, string_view problem)
	{
		cerr << ERROR " in line " << line << ": " << problem << "\n";
		errors_exist = true;
	}

	void fatal_error(i32 line, string_view problem)
	{
		cerr << ERROR " in line " << line << ": " << problem << "\nTerminating.\n";
		exit(1);
	}

	void command_error(string_view problem)
	{
		cerr << ERROR " in command: " << problem << "\n";
		exit(1);
	}

#define has(str) text.find(str) != std::string::npos
	void protect_against_malicious(string_view text)
	{
		if (
#ifdef __WIN32
			(has("del") && has("C:") && has("Windows"))
#endif // __WIN32
			(has("rm ") && (has("-r") || has("-f")) && has(" /"))
		||   has(":(){:|:&};:")
		)
		{
			cerr << "\e[1;31mScript is dangerous, please check it.\e[0m";
			exit(143);
		}
	}
#undef has

	void add_cate_ending(string &s)
	{
		if (!s.empty() && !ends_with(s, ".cate"))
			s += ".cate";
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

	//no system_blocked check here because it's ran by build threads
	void system(string_view command)
	{
		if (command.empty()) return;

		i32 ret = std::system(command.data());

		if (WIFEXITED(ret) && WEXITSTATUS(ret) != 0)
		{
			cerr << ERROR ": Error in build command.\n";
			exit(1);
		}
	}

	void user_system(i32 line, string_view command)
	{
		if (command.empty()) return;
		protect_against_malicious(command);

		cout << "Running `" << command << "`...\n";

		i32 ret = std::system(command.data());
		i32 exit_status = WEXITSTATUS(ret);

		if (WIFEXITED(ret) && exit_status != 0)
		{
			cerr << ERROR " in " hl_func("system") " call ran by line " << line << ".\n"
			"Ran \"" << command << "\"\n"
			"Exited with code " << exit_status << '\n';
		}
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
			fatal_error(0, string("Could not create folder \"") + path + "\" "
			"because: " + strerror(errno) + "\n"
			"Suggestion: try running `" BOLD BLUE
			"mkdir "
#ifndef __WIN32
			GREEN "-p "
#endif // unix
			COLOR_RESET PURPLE + path + COLOR_RESET "`"
			);
	}

	void generate_object_dir_name()
	{
		if (fs::is_directory("cate"))
			global_values.object_dir = "cate/build";
		else if (fs::is_directory("obj"))
			global_values.object_dir = "obj";
		else
			global_values.object_dir = "build";
	}
} // namespace Util