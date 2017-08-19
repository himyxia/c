#include <stdio.h>

int main(int argc, char *argv[])
{
	char name[] = "Jimmy";
	name[1] = 'x';
	printf("%s\n", name);
	return 0;
}
