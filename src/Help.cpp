#include "colors.hpp"
#include <iostream>

void help()
{
	std::cout << BLUE BOLD "Cate " CATE_VERSION "\n"
	"usage: " COLOR_RESET "\tcate " BOLD GREEN "[FLAGS] " PURPLE "[FILENAME]\n\n" COLOR_RESET
	BOLD GREEN "flags:\n"
	hl_flag("-l") ":  list all catefiles in default directory\n"
	hl_flag("-y") ":  install without asking\n"
	hl_flag("-D") ":  disable all " hl_func("system()") " calls in script\n"
	hl_flag("-d") ":  print all commands in script without running them. (dry run)\n"
	hl_flag("-S") ":  smolize even if not set in script\n"
	"\t" BOLD GREEN "-t" hl_var("N") ": set thread count to " PURPLE BOLD "N\n"
	hl_flag("-f") ":  rebuild class's object files; force rebuild\n"
	hl_flag("-v") ":  shows version\n"
	hl_flag("-h") ":  shows help (this)\n"
	;
}