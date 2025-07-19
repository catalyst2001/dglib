#pragma once
#include "dg_libcommon.h"

#define DGCPU_MMX           (1ULL <<  0)
#define DGCPU_SSE           (1ULL <<  1)
#define DGCPU_SSE2          (1ULL <<  2)
#define DGCPU_SSE3          (1ULL <<  3)
#define DGCPU_SSSE3         (1ULL <<  4)
#define DGCPU_SSE4_1        (1ULL <<  5)
#define DGCPU_SSE4_2        (1ULL <<  6)
#define DGCPU_POPCNT        (1ULL <<  7)
#define DGCPU_AVX           (1ULL <<  8)
#define DGCPU_FMA3          (1ULL <<  9)
#define DGCPU_AVX2          (1ULL << 10)
#define DGCPU_BMI1          (1ULL << 11)
#define DGCPU_BMI2          (1ULL << 12)
#define DGCPU_LZCNT         (1ULL << 13)
#define DGCPU_AVX512_F      (1ULL << 14)
#define DGCPU_AVX512_DQ     (1ULL << 15)
#define DGCPU_AVX512_IFMA   (1ULL << 16)
#define DGCPU_AVX512_PF     (1ULL << 17)
#define DGCPU_AVX512_ER     (1ULL << 18)
#define DGCPU_AVX512_CD     (1ULL << 19)
#define DGCPU_AVX512_BW     (1ULL << 20)
#define DGCPU_AVX512_VL     (1ULL << 21)
#define DGCPU_AVX512_VBMI   (1ULL << 22)
#define DGCPU_AVX512_VNNI   (1ULL << 23)
#define DGCPU_AVX512_BITALG (1ULL << 24)
#define DGCPU_AVX512_VPOPCNTDQ (1ULL << 25)
#define DGCPU_SHA           (1ULL << 26)
#define DGCPU_AES           (1ULL << 27)
#define DGCPU_RDRAND        (1ULL << 28)
#define DGCPU_RDSEED        (1ULL << 29)
#define DGCPU_TSX           (1ULL << 30)
#define DGCPU_PREFETCHWT1   (1ULL << 31)
// add more flags as new ISA extensions appear

typedef struct dg_cpu_info_s {
	char     product[64];
	char     vendor[32];
	uint32_t num_physical_processors;
	uint32_t num_logical_processors;
	uint32_t features0;
	uint32_t features1;
} dg_cpu_info_t;

int cpu_get_info(dg_cpu_info_t *pdst);
int cpu_get_current_frequency_ex(double *pdst_freq_hz);