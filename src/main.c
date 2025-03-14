#include "parser.h"

int main(int argc, char* argv[]) {
	puts("Work in progress!");
	return 1;

	Parser parser = {0};
	catel_run(&parser.toks, &parser.vals);

	free(parser.toks.data);
	free(parser.vals.data);
	return 0;
}
