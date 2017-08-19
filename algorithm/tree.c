#include <stdio.h>


struct Node {
	int data;
	struct node *left;
	struct node *right;
};


void insert(int data) {
	if (root == NULL) {
		root = tempNode;
	}else {
		current = root;
		parent = NULL;
	}
}

