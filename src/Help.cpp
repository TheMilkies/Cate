#include "Util.hpp"

void help()
{
	cout << BLUE BOLD "Cate " CATE_VERSION "\n"
	"usage: " COLOR_RESET "\tcate " BOLD GREEN " [FLAGS] " PURPLE "[FILENAME]\n\n" COLOR_RESET
	BOLD GREEN "flags:\n"
	"\t-l" COLOR_RESET ":  list all cate files in default directory\n"
	"\t" BOLD GREEN "-t" highlight_var("N") "  set thread count to " PURPLE BOLD "N\n"
	highlight_flag("-D") " disables all " highlight_func("system()") " calls in script\n"
	highlight_flag("-S") " Smolize even if not set in script\n"
	highlight_flag("-f") " delete everything in class's " highlight_var("build_directory") "; force rebuild\n"
	highlight_flag("-v") " shows version\n"
	highlight_flag("-h") " shows help (this)\n"
	;
}