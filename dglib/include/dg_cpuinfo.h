#pragma once
#include "dg_libcommon.h"

#define DGCPU_MMX           (1ULL <<  0) /*< MMX instruction set */
#define DGCPU_SSE           (1ULL <<  1) /*< SSE instruction set  */
#define DGCPU_SSE2          (1ULL <<  2) /*< SSE2 instruction set */
#define DGCPU_SSE3          (1ULL <<  3) /*< SSE3 instruction set */
#define DGCPU_SSSE3         (1ULL <<  4) /*< SSSE3 instruction set */
#define DGCPU_SSE4_1        (1ULL <<  5) /*< SSE4.1 instruction set */
#define DGCPU_SSE4_2        (1ULL <<  6) /*< SSE4.2 instruction set */
#define DGCPU_POPCNT        (1ULL <<  7) /*< POPCNT instruction set */
#define DGCPU_AVX           (1ULL <<  8) /*< AVX instruction set */
#define DGCPU_FMA3          (1ULL <<  9) /*< FMA3 instruction set */
#define DGCPU_AVX2          (1ULL << 10) /*< AVX2 instruction set */
#define DGCPU_BMI1          (1ULL << 11) /*< BMI1 instruction set */
#define DGCPU_BMI2          (1ULL << 12) /*< BMI2 instruction set */
#define DGCPU_LZCNT         (1ULL << 13) /*< LZCNT instruction set */
#define DGCPU_AVX512_F      (1ULL << 14) /*< AVX-512 Foundation */
#define DGCPU_AVX512_DQ     (1ULL << 15) /*< AVX-512 Doubleword and Quadword Instructions */
#define DGCPU_AVX512_IFMA   (1ULL << 16) /*< AVX-512 Integer Fused Multiply-Add Instructions */
#define DGCPU_AVX512_PF     (1ULL << 17) /*< AVX-512 Prefetch Instructions */
#define DGCPU_AVX512_ER     (1ULL << 18) /*< AVX-512 Exponential and Reciprocal Instructions */
#define DGCPU_AVX512_CD     (1ULL << 19) /*< AVX-512 Conflict Detection Instructions */
#define DGCPU_AVX512_BW     (1ULL << 20) /*< AVX-512 Byte and Word Instructions */
#define DGCPU_AVX512_VL     (1ULL << 21) /*< AVX-512 Vector Length Extensions */
#define DGCPU_AVX512_VBMI   (1ULL << 22) /*< AVX-512 Vector Bit Manipulation Instructions */
#define DGCPU_AVX512_VNNI   (1ULL << 23) /*< AVX-512 Vector Neural Network Instructions */
#define DGCPU_AVX512_BITALG (1ULL << 24) /*< AVX-512 Bit Algorithms */
#define DGCPU_AVX512_VPOPCNTDQ (1ULL << 25) /*< AVX-512 Vector Population Count Double and Quadword */
#define DGCPU_SHA           (1ULL << 26) /*< SHA instruction set */
#define DGCPU_AES           (1ULL << 27) /*< AES instruction set */
#define DGCPU_RDRAND        (1ULL << 28) /*< RDRAND instruction set */
#define DGCPU_RDSEED        (1ULL << 29) /*< RDSEED instruction set */
#define DGCPU_TSX           (1ULL << 30) /*< TSX instruction set */
#define DGCPU_PREFETCHWT1   (1ULL << 31) /*< PREFETCHWT1 instruction set */
// add more flags as new ISA extensions appear

/**
* @brief CPU information structure
*/
typedef struct dg_cpu_info_s {
	char     product[64]; /*< CPU product name */
	char     vendor[32]; /*< CPU vendor name */
	uint32_t num_physical_processors; /*< num physical processors */
	uint32_t num_logical_processors; /*< num logical processors */
	uint32_t features0; /*< feature flags set 0 */
	uint32_t features1; /*< feature flags set 1 */
} dg_cpu_info_t;

/**
* @brief Get CPU information
* @param pdst - Pointer to store the CPU information
* @return 0 on success, -1 on failure
*/
DG_API int cpu_get_info(dg_cpu_info_t *pdst);

/**
* @brief Get current CPU frequency in Hz
* @param pdst_freq_hz - Pointer to store the frequency in Hz
* @return 0 on success, -1 on failure
*/
DG_API int cpu_get_current_frequency_ex(double *pdst_freq_hz);