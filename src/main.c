#include "parser.h"

int main(int argc, char* argv[]) {
	puts("Work in progress!");
	return 1;

	catel_run();
	CateContext ctx = {0};
	cate_run(&ctx, sv_from_cstr(catel.def.x));
	
	return 0;
}
