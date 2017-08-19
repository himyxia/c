#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>
#include <editline/history.h>

int main(int argc, char** argv) {
	
	puts("Lisp Version 0000");
	puts("Press Ctrl+c to Exit\n");

	while(1) {
		char* input = readline("Now> ");
		add_history(input);
		printf("the previous seconds you said: %s\n", input);
		free(input);
	}
	return 0;
}

