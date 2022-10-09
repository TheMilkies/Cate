#include "inc.h"
#include "mmath.h"

int main(int argc, char* argv[])
{
	puts("Simple project!\n\nTesting function from mmath.c: ");
	int result = add(2, 3);
	if(result == 5)
	{
		puts("Passed test!");
	}
	else
	{
		puts("It failed. How?");
	}
	return 0;
}