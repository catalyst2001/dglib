/**
* Purpose: Linear allocator for each thread
* Author:  Deryabin K.
* Created: 12.07.2025
*/
#pragma once
#include "dg_libcommon.h"


/**
* linear memory allocator
*/
#define DGLA_NONE (0)
#define DGLA_AUTORESIZE (1 << 0)
#define DGLA_LESSRESIZE (1 << 1)

typedef struct dg_linalloc_s {
	uint32_t flags;
	size_t   capacity;
	size_t   position;
	uint8_t* pdata;
} dg_hunkalloc_t;

#define la_init_struct(f) { .flags = f, .capacity=0, .position=0, .pdata=NULL }

bool   linalloc_init(dg_hunkalloc_t* pdst, size_t cap);
void   linalloc_deinit(dg_hunkalloc_t* pdst);
void*  linalloc_hunk_alloc(dg_hunkalloc_t* pdst, size_t size);
void   linalloc_reset(dg_hunkalloc_t* pdst);
#define linalloc_is_present(p) ((p)->pdata)

bool   la_present();
bool   la_resize(size_t newsize);
size_t la_get_capacity();
size_t la_get_position();
void*  la_hunk_alloc(size_t size, bool bclear);
void   la_reset();