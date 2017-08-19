#include <stdio.h>

int parent(int i) 
{
	return i/2;
}

int right(int i)
{
	return 2*i + 1;
}

int left(int i)
{
	return 2*i ;
}

void exchange(int *arr[] , int a, int b) 
{
	int tmp;
	tmp = arr[a];
	arr[a] = arr[b];
	arr[b] = tmp;
}

typedef struct {
	int *arr[];
	int heap_size;
	int arr_len;
} heapArr;

heapArr *new_heap_arr(int arr[]) {
	heapArr h = malloc(sizeof(heapArr));
	h->arr = arr;
	h->heap_size  = h->arr_len = len(arr);
	return h;
}

void max_heapify(heapArr *h, int i) 
{
	int l = left(i);
	int r = right(i);

	int largest = 0;
	if( l <= h->heap_size && h->arr[l] > h->arr[i] ) {
		largest = l;
	}else {
		largest = i;
	}

	if( r <= h->heap_size && h->arr[r] > h->arr[i] ) {
		largest = r;
	}

	if (largest != i) {
		exchange(h->arr, largest, i);
		max_heapify(h, largest);
	}
}

void build_max_heap(heapArr *h)
{
	for (int i = h->arr_len / 2; i > 0; i--) {
		max_heapify(h, i);
	}
}

void heap_sort(heapArr *h)
{
	build_max_heap(h);

	for (int i = arr_len; i >= 0; i-- ) {
		exchange(h->arr, 0, i);
		h->heap_size--;
		max_heapify(h, 0)
	}
}

int len(int arr[]) {
	return sizeof(arr) / sizeof(int);
}

void printHeapArr(heapArr h) {
	for (int i = 0; i < h->arr_len; i++ ) 
		if (i == h->arr_len - 1) 
			printf(' ');
		printf("%d ", h->arr[i]);
	}
}

int main(int argc, char** argv) 
{
	int arr[] = {2,5,99,43,12,55,8};
	heapArr *h = new_heap_arr(arr);
	heap_sort(h);
	printHeapArr(h);
	return 0;
}
