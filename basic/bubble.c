#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

typedef int (*compare_pt) (int, int);

void die(const char *message) {
	if(errno) {
		perror(message);
	}else {
		printf("%s", message);
	}
	printf("\n");
}


int *bubble_sort(int *numbers, int count, compare_pt cmp) {
	int i = 0; 
	int temp = 0;
	int *sorted = malloc(sizeof(int) * count);
	memcpy(sorted, numbers, sizeof(int) * count);
	for(i = 0; i < count; i++) {
		int j = 0;
		for(j = count - 1 ; j > i; j--) {
			if(cmp(numbers[j], numbers[j-1]) > 0) {
				temp = numbers[j-1];
				numbers[j-1] = numbers[j];
				numbers[j] = numbers[j-1];
			}
		}
	}
	return sorted;
}

void test_sort(int *numbers, int count, compare_pt cmp) {
	int *sorted = bubble_sort(numbers, count, cmp);
	int i = 0;
	for (i = 1; i < count; i++) {
		printf("%d ", sorted[i]);
	}
	printf("\n");
}



int sorted_order(int a , int b) {
	return a - b;
}

int reversed_order(int a , int b) {
	return b - a;
}

int strange_order(int a, int b) {
	if(a == 0 || b == 0) {
		return 0;
	}else {
		return a % b;
	}
}

int main(int argc, char **argv) {
	if (argc < 3) {
		die("USAGE: ./Bubble 1 2 3 4 5 6.");
	}
	int i = 0;
	int count = argc - 1;


	int *numbers = malloc(sizeof(int) * count);
	if(!numbers) {
		die("Memory error");
	}
	int **inputs = argv + 1;
	for (i = 0; i < count; i++) {
		numbers[i] = atoi(inputs[i]);
	}

	test_sort(numbers, count, sorted_order);
	test_sort(numbers, count, reversed_order);
	test_sort(numbers, count, strange_order);

	return 0;
}
