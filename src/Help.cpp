#include "Util.hpp"

void help()
{
	cout << BLUE BOLD "Cate " CATE_VERSION "\n"
	"usage: " COLOR_RESET "\tcate " BOLD GREEN " [FLAGS] " PURPLE "[FILENAME]\n\n" COLOR_RESET
	BOLD GREEN "flags:\n"
	"\t-l" COLOR_RESET ":  list all catefiles in default directory\n"
	"\t" BOLD GREEN "-t" hl_var("N") "  set thread count to " PURPLE BOLD "N\n"
	hl_flag("-D") " disables all " hl_func("system()") " calls in script\n"
	hl_flag("-S") " smolize even if not set in script\n"
	hl_flag("-f") " delete everything in class's " hl_var("build_directory") "; force rebuild\n"
	hl_flag("-v") " shows version\n"
	hl_flag("-h") " shows help (this)\n"
	;
}