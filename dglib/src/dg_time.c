#include "dg_time.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

LARGE_INTEGER global_freq;

uint64_t dg_get_perf_frequency()
{
  LARGE_INTEGER freq;
  QueryPerformanceFrequency(&freq);
  return freq.QuadPart;
}

uint64_t dg_get_perf_counter()
{
  LARGE_INTEGER counter;
  QueryPerformanceCounter(&counter);
  return counter.QuadPart;
}

double dg_get_time_sec()
{
  LARGE_INTEGER counter;
  QueryPerformanceCounter(&counter);
  return counter.QuadPart / (double)global_freq.QuadPart;
}

void time_init()
{
  QueryPerformanceFrequency(&global_freq);
}

void time_deinit()
{
}

#else

#endif