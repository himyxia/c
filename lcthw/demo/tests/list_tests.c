#include "list.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

int tests_run;

char *test_create() {
	List *l = List_create();
	if(l == NULL) {
		fprintf(stderr, "[ERROR] (%s:%d: errno: %s)", "failed to create list.");
		return "failed to create list";
	}
	return NULL;
}

char *all_tests() {
	char *message = NULL;

	fprintf(stderr, "DEBUG %s:%d: \n----%s", __FILE__, __LINE__, "test_create");
	message = test_create();
	tests_run++;
	if (message) {
		return message;
	}

	/*
	fprintf(stderr, "DEBUG %s:%d: \n----%s", "test_push_pop");
	message = test_push_pop();
	tests_run++;
	if (message) {
		return message;
	}

	fprintf(stderr, "DEBUG %s:%d: \n----%s", "test_unshift");
	message = test_unshift();
	tests_run++;
	if (message) {
		return message;
	}

	fprintf(stderr, "DEBUG %s:%d: \n----%s", "test_remove");
	message = test_remove();
	tests_run++;
	if (message) {
		return message;
	}

	fprintf(stderr, "DEBUG %s:%d: \n----%s", "test_shift");
	message = test_shift();
	tests_run++;
	if (message) {
		return message;
	}

	fprintf(stderr, "DEBUG %s:%d: \n----%s", "test_destory");
	message = test_destory();
	tests_run++;
	if (message) {
		return message;
	}
	*/
	return NULL;
}

int main(int argc, char* argv[]) {
	char *message = NULL;
	argc = 1;

	fprintf(stderr, "DEBUG %s:%d: -----RUNNING: %s\n", __FILE__, __LINE__, argv[0]);

	printf("----\nRUNNING: %s\n", argv[0]);
	char *result = all_tests();
	if (result != 0) {
		printf("FAILED: %s\n", result);
	}else {
		printf("ALL TESTS PASSED\N");
	}
	printf("Tests_run: %d\n", tests_run);
	exit(result != 0);
}
