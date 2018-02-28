#include<stdio.h>

int myglob = 42;

int ml_func(int a, int b) {
	myglob += a;
	return b + myglob;
}
