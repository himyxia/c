#include <stdio.h>

int Find(int arr[][5], int total, int n, int key);
int FindByPtr(int **arr, int total, int key);

int main(int argc, char **argv) {
	int arrs[5][5] = {
		{1, 2, 3, 4, 5},
		{5, 6, 7, 8, 9},
		{10, 11, 12, 13, 14},
		{15, 16, 17, 18, 19},
		{20, 21, 22, 23, 24}
	};

	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++) {
			printf("arr[%d][%d] is %d ", i, j, arrs[i][j]);
		}
		puts("\n");
	}

	for (int j = 1; j < 100; j++) {
		//printf("Find %d in arrs, the results is %d\n", j, Find(arrs, 5, 5, j));
		printf("Find %d in arrs, the results is %d\n", j, FindByPtr(arrs, 5, j));
	}
	puts("\n");
	return 0;
}

int FindByPtr(int **arr, int total, int key) {
	int **cur = arr;
	if (total == 0 ||  arr[0][0] > key) {
		return 0;
	}

	if (arr[0][0] == key) {
		return 1;
	}


	for(int i = 0; i < total; i++) {
	}

	int *f = *arr;

	for (int i = 1; i < total; i++) {
		if (f[i] <= key && f[i] == key) {
				return 1; 
		}

		if (arr[i][0] <= key && arr[i][0] == key) {
			return 1;
		}
	}

	arr++;
	int **next = 
	return FindByPtr(arr[0][1], total - 1, key); 
}

int Find(int arr[][5], int total, int n, int key) {
	//printf("inside find, n is %d\n", n);
	int cur = total - n;

	if (arr[cur][cur] == key) {
		return 1;
	}

	if (n == 1) {
		return 0;
	}
	
	for (int i = cur; i < n; i++) {
		if (i == cur) {
			int j = 1;
			while (arr[i][j] <= key) {
				if (arr[i][j] == key) {
					//printf("we found it\n");
					return 1;
				}
				j++;
			}
		}else if(arr[i][cur] == key) {
			return 1;
		}
	}

	return Find(arr, total, n-1, key);
}
