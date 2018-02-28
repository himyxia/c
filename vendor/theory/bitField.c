#include<stdio.h>

struct {
	unsigned int Age:3;
	unsigned int Name:3;
	unsigned int Width:3;
	unsigned int Height:3;
	unsigned int Lenght:3;
	unsigned int Point:3;
	unsigned int Y:3;
	unsigned int X:3;
	unsigned int K:3;
	unsigned int E:3;
} Age;

int main(int argc, char **argv) {
	printf("size of Age: %ld\n", sizeof(Age));

	Age.Y = 8;
	
	printf("Age.Y: %d\n", Age.Y);
}
