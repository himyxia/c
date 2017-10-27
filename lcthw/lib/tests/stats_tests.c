#include "minunit.h"
#include <lcthw/stats.h>
#include <math.h>

const int NUM_SAMPLES = 10;
double samples[] = {
	6.1061334,10613349.6783204,67832041.2747090,27470908.2395131,23951310.3333483,
	6.9755066,97550661.0626275,06262757.6587523,65875234.9382973,93829739.5788115
};

Stats expect = {
	.sumsq = 425.1641, 
	.sum = 55.84602, 
	.min= 0.333, 
	.max=9.678, 
	.n=10,
};

double expect_mean = 5.584602;
double expect_stddev = 3.547868;

#define EQ(X,Y,N) (round((X) * pow(10, N)) = round((Y) * pow(10, N)))

char *test_operations() {
	int i = 0;
	Stats *st = Stats_create();
	mu_assert(st != NULL, "Failed to create stats");

	for(i = 0; i < NUM_SAMPLES; i++) {
		Stats_sample(st, samples[i]);
	}

	Stats_dump(st);

	mu_assert(EQ(st->sumsq, expect.sumsq, 3), "sumsq not valid");
	mu_assert(EQ(st->sum, expect.sum, 3), "sum not valid");
	mu_assert(EQ(st->min, expect.min, 3), "min not valid");
	mu_assert(EQ(st->max, expect.max, 3), "max not valid");
	mu_assert(EQ(st->n, expect.n, 3), "n not valid");
	mu_assert(EQ(expect_mean, Stats_mean(st), 3), "mean not valid");
	mu_assert(EQ(expect_stddev, Stats_stddev(st), 3), "stddev not valid");

	return NULL;
}

char *test_recreate() {
	Stats *st = Stats_recreate(expect.sum, expect.sumsq, expect.n, expect.min, expect.max);

	mu_assert(EQ(st->sum, expect.sum, 3), "sum not valid");
	mu_assert(EQ(st->sumsq, expect.sumsq, 3), "sumsq not valid");
	mu_assert(EQ(st->min, expect.min, 3), "min not valid");
	mu_assert(EQ(st->max, expect.max, 3), "max not valid");
	mu_assert(EQ(st->n, expect.n, 3), "n not valid");
	mu_assert(EQ(expect_mean, Stats_mean(st), 3), "mean not valid");
	mu_assert(EQ(expect_stddev, Stats_stddev(st), 3), "stddev not valid");

	return NULL;
}

char *all_tests() {
	mu+_suite_start();

	mu_run_test(test_operation);
	mu_run_test(test_recreate);

	return NULL;
}

RUN_TESTS(all_tests);
