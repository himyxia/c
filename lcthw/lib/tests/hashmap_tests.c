#include "minunit.h"
#include <assert.h>
#include <lcthw/bstrlib.h>
#include <lcthw/hashmap.h>

Hashmap *map = NULL;
static int traverse_called = 0;
struct tagbstring test1 = bsStatic("test data 1");
struct tagbstring test2 = bsStatic("test data 2");
struct tagbstring test3 = bsStatic("test data 3");

struct tagbstring expect1 = bsStatic("THE VALUE 1");
struct tagbstring expect2 = bsStatic("THE VALUE 2");
struct tagbstring expect3 = bsStatic("THE VALUE 3");

static int traverse_good_cb(HashmapNode *node) {
	debug("Key: %s", bdata((bstring)node->key));
	traverse_called++;
	return 0;
}

static int traverse_fail_cb(HashmapNode *node) {
	debug("Key: %s", bdata((bstring)node->key));
	traverse_called++;

	if(traverse_called == 2) {
		return 1;
	} 
	return 0;
}

char *test_destory() {
	Hashmap_destory(map);
	return NULL;
}

char *test_delete() {
	bstring deleted = (bstring)Hashmap_delete(map, &test1);
	mu_assert(deleted != NULL, "Got NULL on delete");
	mu_assert(deleted == &expect1, "should got test1");

	bstring result = Hashmap_get(map, &test1);
	mu_assert(result == NULL, "should delete");

	deleted = (bstring)Hashmap_delete(map, &test2);
	mu_assert(deleted != NULL, "GOT null on delete");
	mu_assert(deleted == &expect2, "should get test2");
	result = Hashmap_get(map, &test2);
	mu_assert(result == NULL, "should delete.");

	deleted = (bstring)Hashmap_delete(map, &test3);
	mu_assert(deleted != NULL, "GOT null on delete");
	mu_assert(deleted == &expect3, "should get test3");
	result = Hashmap_get(map, &test3);
	mu_assert(result == NULL, "should delete.");

	return NULL;
}

char *test_traverse() {
	int rc = Hashmap_traverse(map, traverse_good_cb);
	mu_assert(rc == 0, "Failed to traverse");
	mu_assert(traverse_called == 3, "Wrong count traverse");

	traverse_called = 0;
	rc = Hashmap_traverse(map, traverse_fail_cb);
	mu_assert(rc == 1, "Failed to traverse");
	mu_assert(traverse_called == 2, "Wrong count traverse");

	return NULL;
}

char *test_get_set() {
	int rc = Hashmap_set(map, &test1, &expect1);
	if(!(rc==0)) {
		fprintf(stderr, "[ERROR] (%s:%d:errno:%s) FAILED to set &test1.\n", __FILE__, __LINE__, errno == 0 ? "NONE" : strerror(errno));
		return "Failed to set &test1";
	}

	bstring result = Hashmap_get(map, &test1);
	if(!(result == &expect1)) {
		fprintf(stderr, "[ERROR] (%s:%d:errno:%s) Wrong value for test1.\n", __FILE__, __LINE__, errno==0 ? "NONE": strerror(errno));
		return "Wrong value for tests1";
	}

	rc = Hashmap_set(map, &test2, &expect2);
	mu_assert(rc==0, "Failed to set test2");
	result = Hashmap_get(map, &test2);
	mu_assert(result == &expect2, "Wrong value for test2.");

	rc = Hashmap_set(map, &test3, &expect3);
	mu_assert(result == &expect2, "wrong value for test3.");

	return NULL;
}

char *test_create() {
	map = Hashmap_create(NULL, NULL);
	if(!(map != NULL)) {
		fprintf(stderr, "[ERROR] (%s:%d: errno: %s) Failed to create map. \n", __FILE__, __LINE__, errno == 0 ? "NONE" : strerror(errno));
		return "Failed to create map";
	}
	return NULL;
}


char *all_tests() {
	char *message = NULL;

	debug("\n---%s", " test_create");
	message = test_create();
	tests_run++;
	if(message) 
		return message;

	debug("\n---%s", " test_get_set");
	message = test_get_set();
	tests_run++;
	if(message) 
		return message;

	debug("\n---%s", " test_traverse");
	message = test_traverse();
	tests_run++;
	if(message) 
		return message;

	debug("\n---%s", " test_delete");
	message = test_delete();
	tests_run++;
	if(message) 
		return message;

	debug("\n---%s", " test_destory");
	message = test_destory();
	tests_run++;
	if(message) 
		return message;

	return NULL;
}

int main(int argc, char *argv[]) {
	argc = 1;
	debug("----RUNNING %s", argv[0]);
	printf("----\nRUNNING %s\n", argv[0]);
	char *result = all_tests();
	if(result != 0) {
		printf("FAILED: %s\n", result);
	}else {
		printf("ALL TESTS PASSED\n");
	}
	printf("Tests run: %d\n", tests_run);

	exit(result != 0);
}

int tests_run;
