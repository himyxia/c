#include "minunit.h"
#include <lcthw/darray.h>

static DArray *array = NULL;
static int *val1 = NULL;
static int *val2 = NULL;

char *test_create() {
	array = DArray_create(sizeof(int), 100);
	mu_assert(array != NULL, "DArray_create failed.");
	mu_assert(array ->contents != NULL, "contents are wrong in darray.");
	mu_assert(array ->end == 0, "end isn't at the right spot");
	mu_assert(array ->element_size == sizeof(int), "element size is wrong.");
	mu_assert(array ->max == 100, "wrong max lenght on initial size.");

	return NULL;
}

