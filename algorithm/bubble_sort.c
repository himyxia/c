#include <stdio.h>
#include <string.h>

void swap(int a[], int i, int j) {
	int t = a[i];
	a[i] = a[j];
	a[j] = t;
}

int len(char *a) {
	return strlen(a);
}

void append(char s[], char c) {
	int len = strlen(s);
	s[len] = c;
	s[len+1] = '\0';
}

void sort(char a[]) {
	for(int i = 1; i < len(a); i++) {
		printf("should enter \n");
		char key = a[i];
		int j = 0;
		for (j = i - 1; j >= 0 && (key < a[j]); j--) {
				a[j+1] = a[j];
		}
		a[j+1] = key;
	}
}

int exist(char a, char *b) {
	for(int j = 0; j < len(b); j++) {
		if (a == b[j]) {
			return 1;
		}
	}
	return 0;
}



char *set_diff(char *a, char *b) {
	sort(a);
	sort(b);

	char *ret = "";

	for(int i = 0; i < len(a); i++) {
		if (a[i] < b[0]) {
			append(ret, a[i]);
		}

		if (!exist(a[i], b)) {
			append(ret, a[i]);
		}

		if (a[i] > b[len(b)-1]) {
			append(ret, a[i]);
		}

	}
	return ret;
}

int main(int argc, char* argv[]) {
	char *a = "abcdefghijk";
	char *b = "defjk";

	char *c = set_diff(a, b);

	//char a[] = {'a', 'b', 'c', 'd'};

	//printf("%d \n", sizeof(a)/sizeof(char));

	//insert_sort(a);
	for (int i = 0; i < len(c); i++) {
		printf("%d: %c ", i, c[i]);
		puts("\n");
	}
}
