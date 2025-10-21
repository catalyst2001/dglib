#pragma once
#include "dg_libcommon.h"

DG_API uint64_t dg_get_perf_frequency();
DG_API uint64_t dg_get_perf_counter();
DG_API double   dg_get_time_sec();

typedef struct dg_timer_s {
	double start_time;
	double end_time;
	double elapsed_time;
} dg_timer_t;

static inline void dg_timer_start(dg_timer_t* ptimer) {
	ptimer->start_time = dg_get_time_sec();
	ptimer->end_time = 0.0;
	ptimer->elapsed_time = 0.0;
}

static inline void dg_timer_stop(dg_timer_t* ptimer) {
	ptimer->end_time = dg_get_time_sec();
	ptimer->elapsed_time = ptimer->end_time - ptimer->start_time;
}

#define timer_get_elapsed(ptimer) ((ptimer)->elapsed_time)
#define timer_get_elapsed_ms(ptimer) ((ptimer)->elapsed_time * 1000.0)