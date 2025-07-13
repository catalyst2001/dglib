/**
* Purpose: Linear allocator for each thread
* Author:  Deryabin K.
* Created: 12.07.2025
*/
#pragma once
#include "dg_libcommon.h"

bool   la_present();
bool   la_resize(size_t newsize);
size_t la_get_capacity();
size_t la_get_position();
void*  la_hunk_alloc(size_t size, bool bclear);
void   la_reset();