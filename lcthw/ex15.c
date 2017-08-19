#include <stdio.h>

int main(int argc, char *argv[]) 
{
	int ages[] = {23, 43, 12, 89, 2};
	char *name[] = {
		"Alan", "Frank",
		"Mary", "John", "Lisa"
	};

	int count = sizeof(ages) / sizeof(int);
	int i = 0;

	for(i = count-1; i >= 0; i--) {
		printf("%s has age %d \n",
				name[i], ages[i]);
	}
	printf("-----\n");

	int *cur_age = ages;
	char **cur_name = name;

	for(i = count-1; i >= 0; i--) {
		printf("%s has age %d \n",
				*(cur_name+i), *(cur_age+i));

		printf("%s has age %d \n",
				*(name+i), *(ages+i));
	}
	printf("-----\n");


	for(i = count-1; i >= 0; i--) {
		printf("%s has age %d \n",
				cur_name[i], cur_age[i]);
	}
	printf("-----\n");

	return 0;
}
