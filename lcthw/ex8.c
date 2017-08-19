#include <stdio.h>

int main(int argc, char *argv[]) 
{
	int areas[] = {10, 12, 13, 14, 20};
	char name[] = "Zed";
	char full_name[] = {
		'Z', 'e', 'd',
		' ', 'A', '.', ' ',
		'S', 'h', 'a', 'w','\0'
	};
	printf("the size of an int: %ld\n", sizeof(int));
	printf("the size of areas: %ld\n", sizeof(areas));
	printf("the number of int in areas: %ld\n", sizeof(areas) / sizeof(int));
	printf("the first element in areas is %d, the 2nd is %d \n", areas[0], areas[1]);

	printf("the size of a char: %ld\n", sizeof(char));
	printf("the size of name: %ld\n", sizeof(name));
	printf("the number of chars in name: %ld\n", sizeof(name) / sizeof(char));

	printf("the size of full_name: %ld\n", sizeof(full_name));
	printf("the number of chars in full_name: %ld\n", sizeof(full_name) / sizeof(char));

	printf("name= \"%s\" and full_name=\"%s\"\n", name, full_name);

	// credits
	areas[0] = 48;
	printf("the first element in areas is %d.\n", areas[0]);

	areas[0] = name[1];
	printf("the first element in areas is %d.\n", areas[0]);

	name[3] = 77;
	full_name[3] = 77;
	printf("name= \"%s\" and full_name=\"%s\"\n", name, full_name);

	return 0;
}
