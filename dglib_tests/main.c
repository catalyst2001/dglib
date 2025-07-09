#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dg_darray.h"
#include "dg_list.h"
#include "dg_queue.h"

#define DG_MEMNOVERRIDE
#include "dg_alloc.h"

void clear_term()
{
	system(
#if defined(_WINDOWS) || defined(_MSC_VER)
		"cls"
#else
		"clear"
#endif
	);
}

/* default impl */
void* mem_alloc_default(void* poldmem, size_t size, uint32_t flags) {
	void* ptr;
	if (!size)
		return NULL;

	ptr = malloc(size);
	if (!ptr)
		return NULL;

	if (flags & DGMM_CLEAR)
		memset(ptr, 0, size);

	if (poldmem && (flags & DGMM_COPY))
		memcpy(ptr, poldmem, size);

	return ptr;
}

int mem_free_default(void* pmem) {
	if (pmem) {
		free(pmem);
		return DGERR_NONE;
	}
	return DGERR_INVALID_PARAM;
}

void* mem_alloc_debug_default(void* poldmem, size_t size, uint32_t flags, const char* pfile, int line) {
	void* ptr = mem_alloc_default(poldmem, size, flags);
	if (!ptr) {
		fprintf(stderr, "mem_alloc_debug_default(): allocation failed! File: %s Line: %d\n", pfile, line);
		return NULL;
	}
	return ptr;
}

int mem_free_debug_default(void* pmem, const char* pfile, int line) {
	int status = mem_free_default(pmem);
	if (status != DGERR_NONE)
		fprintf(stderr, "mem_free_debug_default(): free failed! status: %d  File: %s  Line: %d\n", 
			status, pfile, line);

	return status;
}

dg_memmgr_dt_t dt = {
	.mem_alloc = mem_alloc_default,
	.mem_alloc_debug = mem_alloc_debug_default,
	.mem_free = mem_free_default,
	.mem_free_debug = mem_free_debug_default
};
dg_memmgr_dt_t* gpmmdt = &dt;

bool test_list()
{
	dg_queue_t queue = queue_init(int, 16);
	if (!queue_alloc(&queue)) {
		printf("queue alloc failed!\n");
		return false;
	}




	return true;
}

bool test_list()
{
	dg_list_t list = list_init(int);
	for (int i = 0; i < 10; i++)
		list_add_back(&list, &i);

	int front_value = 1000;
	list_add_front(&list, &front_value);

	dg_llnode_t* pnode = list.pbegin;
	while (pnode) {
		printf("%d ", *((int*)pnode->data));
		pnode = pnode->pnext;
	}

	printf("elem at 0: %d\n", *((int *)list_get_at(&list, 0)->data));
	printf("elem at 1: %d\n", *((int *)list_get_at(&list, 1)->data));

	list_free(&list);
	return true;
}

bool test_darray3d()
{
	size_t x, y, z;
	darray3d_t array3d = darray3d_init(10, 10, 10, int);
	if (!darray3d_alloc(&array3d, true)) {
		printf("3d array allcoation failed!\n");
		return false;
	}

	for (y = 0; y < array3d.dim1; y++) {
		for (x = 0; x < array3d.dim0; x++) {
			for (z = 0; z < array3d.dim2; z++) {
				if (x == y && y == z) {
					darray3d_get(&array3d, x, y, z, int) = 1;
					darray3d_get(&array3d, array3d.dim0 - 1 - x, y, z, int) = 2;
					darray3d_get(&array3d, x, array3d.dim1 - 1 - y, z, int) = 3;
					darray3d_get(&array3d, x, y, array3d.dim2 - 1 - z, int) = 4;
				}
			}
		}
	}

	clear_term();
	printf("--------- printing 3d array by Z (depth) layers ---------\n");
	for (z = 0; z < array3d.dim2; z++) {
		printf("layer %zd of %zd\n", z+1, array3d.dim2);
		for (y = 0; y < array3d.dim1; y++) {
			for (x = 0; x < array3d.dim0; x++) {
				printf("%c ", "-1234"[darray3d_get(&array3d, x, y, z, int)]);
			}
			printf("\n");
		}
		printf("next layer? (press any key): ");
		(void)getchar();
		clear_term();
	}
	darray3d_free(&array3d);
	int yn = 0;
	printf("this result is correct? (1/0): ");
	scanf_s("%d", &yn);
	return !!yn;
}

bool test_darray2d()
{
	size_t x, y;
	darray2d_t array2d = darray2d_init(10, 10, int);
	if (!darray2d_alloc(&array2d, true)) {
		printf("allocation failed!\n");
		return false;
	}

	/* fill */
	for (y = 0; y < array2d.cols; y++) {
		for (x = 0; x < array2d.rows; x++) {
			if (x == y) {
				darray2d_get(&array2d, x, y, int) = 1;
				darray2d_get(&array2d, array2d.cols - 1 - x, y, int) = 2;
				continue;
			}
		}
	}

	/* print */
	printf("----------------------------\n");
	for (y = 0; y < array2d.cols; y++) {
		for (x = 0; x < array2d.rows; x++) {
			printf("%c ", "-01"[darray2d_get(&array2d, x, y, int)]);
		}
		printf("\n");
	}
	printf("----------------------------\n");

	darray2d_free(&array2d);

	int yn = 0;
	printf("this result is correct? (1/0): ");
	scanf_s("%d", &yn);
	getchar();//read '\n'
	return !!yn;
}

bool test_darray1d()
{
	darray_t dynarray = darray_init(int, 10, 10, 0);
	for (int i = 0; i < 10; i++) {
		darray_push_back(&dynarray, &i);
		printf("%d ", i);
	}
	printf("\n");

	printf("PRE size: %zd  cap: %zd  reserve: %zd\n", 
		dynarray.size,
		dynarray.capacity,
		dynarray.reserve
	);

	printf("adding last element for change cap with reserve\n");
	int lastelem = 999;
	darray_push_back(&dynarray, &lastelem);
	darray_push_back(&dynarray, &lastelem);
	printf("POST size: %zd  cap: %zd  reserve: %zd\n",
		dynarray.size,
		dynarray.capacity,
		dynarray.reserve
	);

	printf("------ elems output -------\n");
	for (int i = 0; i < darray_get_size(&dynarray); i++)
		printf("  [%d] %d\n", i, darray_get(&dynarray, i, int));

	darray_free(&dynarray);
	return true;
}

#define RUN_TEST(func, failmsg) do {\
	if(!func()) {\
		printf(failmsg "\n");\
		return 1;\
	}\
} while (0.0);

int main()
{
	RUN_TEST(test_darray1d, "1D array testing failed!")
	RUN_TEST(test_darray2d, "2D array testing failed!")
	RUN_TEST(test_darray3d, "3D array testing failed!")
	RUN_TEST(test_list, "list testing failed!")

	return 0;
}