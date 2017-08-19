#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) 
{
	int i = argc-1;
	while (i >= 0) {
		printf("arg %d: %s\n", i, argv[i]);
		i--;
	}

	char *states[] = {
		"California", "Oregon",
		"Washington", "Texas"
	};

	int num_states = 4;
	i = num_states-1;
	while (i >= 0) {
		printf("state: %d %s\n", i, states[i]);
		i--;
	}

	i = 0;
	while (i < argc) {
		if (i == num_states )
			break;

		states[i] = argv[i];
		//strcpy(states[i], argv[i]);
		i++;
	}

	argv[0] = realloc(argv[0], sizeof(char)*(8));
	argv[0][7] = 'J';

	i = num_states-1;
	while (i >= 0) {
		printf("state: %d %s\n", i, states[i]);
		i--;
	}
	return 0;
}
