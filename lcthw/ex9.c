#include <stdio.h>

int main(int argc, char *argv[]) 
{
	char jimmy[5] = {'J', 'I', 'M', 'M', 'Y'};

	jimmy[0] = 'X';
	jimmy[1] = 'X';
	jimmy[2] = 'X';
	jimmy[3] = 'I';
	jimmy[4] = 'I';
	jimmy[5] = 'I';
	jimmy[6] = 'I';
	jimmy[7] = 'I';

	printf("Jimmy: %s\n", jimmy);
	printf("size of Jimmy: %ld\n", sizeof(jimmy));


	int intArr[] = {12, 22, 34};
	printf("using array as pointer -> intArr: %d %d %d\n", *(intArr+0), *(intArr+1), *(intArr+2));
	printf(intArr);
	printf("**************\n");

//	intArr[2] = 55;
//	printf("intArr: \n", intArr[0], intArr[1], intArr[2]);

	int numbers[4] = {0};
	char name[4] = {'a'};

	printf("numbers: %d %d %d %d\n", 
			numbers[0], numbers[1],
			numbers[2], numbers[3]);

	printf("name each: %c %c %c %c\n", 
			name[0], name[1],
			name[2], name[3]);
	
	printf("name: %s\n", name);

	numbers[0] = 1;
	numbers[1] = 2;
	numbers[2] = 3;
	numbers[3] = 4;

	name[0] = 'Z';
	name[1] = 'e';
	name[2] = 'd';
	name[3] = '\0';

	printf("numbers: %d %d %d %d\n", 
			numbers[0], numbers[1],
			numbers[2], numbers[3]);

	printf("name: %d %d %d %d\n", 
			name[0], name[1],
			name[2], name[3]);

	printf("name: %s\n", name);

	// transform name into integer
	int rel = name[0] << 24 | name[1] << 16 | name[2] <<8;
	//unsigned int rel = 90<<24;
	printf("name into integer: %d.\n", rel);

	name[0] = 23;
	name[1] = 24;
	name[2] = 25;
	name[3] = 0;

	printf("name: %d %d %d %d.\n", 
			name[0], name[1],
			name[2], name[3]);

	char *another = "Zed";
	//*another = 'J';
	printf("another: %s\n", another);

	printf("another each: %c %c %c %c\n",
			another[0], another[1],
			another[2], another[3]);
	
	numbers[0] = 'J';
	numbers[1] = 'i';
	numbers[2] = 'm';
	numbers[3] = 'm';
	printf("numbers: %d %d %d %d.\n",
			numbers[0], numbers[1],
			numbers[2], numbers[3]);

	return 0;
}

