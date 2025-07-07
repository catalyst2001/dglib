/**
* DGLIB
* Copyright (c) 2025 DiggyGods team
* 
* Author: Deryabin K. "catalyst"
*/
#pragma once
#include "dg_libcommon.h"

typedef struct dg_da_s {
	size_t   elemsize;
	size_t   capacity;
	size_t   reserve;
	size_t   size;
	uint8_t* pdata;
} dg_da_t;

#define dg_da_init_default(type) {\
	.elemsize = sizeof(type),\
	.capacity = 1,\
	.reserve = 1,\
	.size = 0,\
	.pdata=NULL\
}

#define dg_da_init(type, cap, res, siz) {\
	.elemsize = sizeof(type),\
	.capacity = cap,\
	.reserve = res,\
	.size = siz,\
	.pdata=NULL\
}

#define dg_da_get_size(p) ((p)->size)
#define dg_da_get_capacity(p) ((p)->capacity)
#define dg_da_get_reserve(p) ((p)->reserve)
#define dg_da_get_elemsize(p) ((p)->elemsize)
#define dg_da_set_reserve(p, newreserve)\
	assert(newreserve != 0 && "newreserve was zero! this value is invalid");\
	(p)->reserve = newreserve;

void*   dg_da_get_ex(dg_da_t *pdarray, size_t idx);
#define dg_da_get(p, idx, type) *((type *)dg_da_get_ex(p, idx))
#define dg_da_get_data_ptr(p) (p)->pdata
#define dg_da_get_data_ptr_as(p, type) ((type *)(p)->pdata)
int     dg_da_resize(dg_da_t *pdarray, size_t newsize);
int     dg_da_realloc(dg_da_t *pdarray, size_t newcap);
int     dg_da_shrink_to_fit(dg_da_t* pdarray);
void    dg_da_move(dg_da_t* pdarray, dg_da_t *pdarraysrc);
int     dg_da_copy(dg_da_t* pdarray, const dg_da_t *pdarraysrc);
void    dg_da_insert_from(dg_da_t* pdarray, const dg_da_t *pdarraysrc, size_t from, size_t count);
void    dg_da_clear(dg_da_t* pdarray);
void    dg_da_reset(dg_da_t* pdarray);
bool    dg_da_free(dg_da_t* pdarray);
bool    dg_da_push_back(dg_da_t* pdarray, const void *psrc);
bool    dg_da_push_front(dg_da_t* pdarray, const void* psrc);
bool    dg_da_pop_back(void* pdst, dg_da_t* pdarray);
bool    dg_da_pop_front(void* pdst, dg_da_t* pdarray);
