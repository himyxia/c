#include <stdio.h>

int main(int argc, char *argv[])
{
	int i = 0;
	if(argc == 2) {
		printf("you have only one argument, you're suck\n");
	}else if(argc  > 2 && argc < 4) {
		printf("Here is your arguments:\n");

		for(i = 0; i < argc; i++) 
			printf("%s\n", argv[i]);
		
		printf("\n");
	}else if(argc == 1) {
		printf("you have no arguments, you're suck\n");
	}else {
		printf("you have too many arguments, you're suck\n");
	}
	return 0;
}
