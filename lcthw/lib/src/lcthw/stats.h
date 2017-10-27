#ifndef lcthw_stats_h
#define lcthw_stats_h

typedef struct Stats {
	double sum;
	double sumsq;
	unsigned long n;
	double min;
	double max;
} Stats;

Stats *Stats_recreate(double sum, double sumsq, unsigned long n, double min, double max);

Stats *Stats_create();

doubles Stats_mean(Stats *st);

doubles Stats_stddev(Stats *st);

void Stats_sample(Stats *st, double s);

void Stats_dump(Stats *st);

#endif
