#include "dg_cpuinfo.h"
#include "dg_time.h"
#include "dg_thread.h"
#include <intrin.h>

int cpu_get_info(dg_cpu_info_t* pdst)
{
  if (!pdst)
    return DGERR_INVALID_PARAM;

  int regs[4];
  // Leaf 0: get vendor ID and max standard leaf
  __cpuid(regs, 0);
  int maxStdLeaf = regs[0];
  // Vendor ID string
  memcpy(pdst->vendor + 0, &regs[1], 4);
  memcpy(pdst->vendor + 4, &regs[3], 4);
  memcpy(pdst->vendor + 8, &regs[2], 4);
  pdst->vendor[12] = '\0';

  // Leaf 0x80000000: max extended leaf
  __cpuid(regs, 0x80000000);
  unsigned int maxExtLeaf = (unsigned int)regs[0];

  // Brand string (product) from extended leaves 0x80000002..4
  if (maxExtLeaf >= 0x80000004) {
    char* p = pdst->product;
    __cpuid((int*)(p + 0), 0x80000002);
    __cpuid((int*)(p + 16), 0x80000003);
    __cpuid((int*)(p + 32), 0x80000004);
    pdst->product[48] = '\0';
  }
  else {
    pdst->product[0] = '\0';
  }

  // Features bits from leaf 1
  if (maxStdLeaf >= 1) {
    __cpuid(regs, 1);
    pdst->features0 = (uint32_t)regs[3]; // EDX
    pdst->features1 = (uint32_t)regs[2]; // ECX
    // Logical processors per package: EBX[23:16]
    pdst->num_logical_processors = (regs[1] >> 16) & 0xFF;
  }
  else {
    pdst->features0 = pdst->features1 = 0;
    pdst->num_logical_processors = 1;
  }

  // Number of physical cores: leaf 4, subleaf 0: EAX[31:26] + 1
  if (maxStdLeaf >= 4) {
    // __cpuidex available for subleaf
    __cpuidex(regs, 4, 0);
    uint32_t cores = ((uint32_t)regs[0] >> 26) + 1;
    pdst->num_physical_processors = cores;
  }
  else {
    pdst->num_physical_processors = pdst->num_logical_processors;
  }
  return DGERR_SUCCESS;
}

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

int cpu_get_current_frequency_ex(double* pdst_freq_hz)
{
#ifdef _WIN32
  /* restrict changing affinity! manual set */
  DWORD_PTR affinity = 1;
  DWORD_PTR old_affinity = SetThreadAffinityMask(GetCurrentThread(), affinity);
#endif
  uint64_t perf_freq = dg_get_perf_frequency();
  uint64_t perf_start = dg_get_perf_counter();
  uint64_t perf_end;
  uint64_t cycles_start;
  uint64_t cycles_end;
  uint64_t cycles_diff;
  double   elapsed_sec;

  cycles_start = __rdtsc();
  dg_delay_ms(100);
  perf_end = dg_get_perf_counter();
  cycles_end = __rdtsc();
  elapsed_sec = ((double)(perf_end - perf_start) / ((double)perf_freq));
  cycles_diff = cycles_end - cycles_start;
  *pdst_freq_hz = cycles_diff / elapsed_sec;
#ifdef _WIN32
  SetThreadAffinityMask(GetCurrentThread(), old_affinity);
#endif
  return DGERR_SUCCESS;
}
