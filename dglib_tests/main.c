#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "dg_darray.h"
#include "dg_list.h"
#include "dg_queue.h"
#include "dg_cpuinfo.h"
#include "dg_dt.h"
#include "dg_handle.h"
#include "dg_bitvec.h"
#include "dg_thread.h"
#include "dg_threadpool.h"

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
		return DGERR_SUCCESS;
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
	if (status != DGERR_SUCCESS)
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

bool test_cpuinfo()
{
	dg_cpu_info_t info;
	if (cpu_get_info(&info) != DGERR_SUCCESS) {
		printf("cpu_get_info() failed\n");
		return false;
	}
	printf("---- cpu info ----\n");
	printf("product: \"%s\"\n", info.product);
	printf("vendor: \"%s\"\n", info.vendor);
	printf("num phys processors: %d\n", info.num_physical_processors);
	printf("num logical processors: %d\n", info.num_logical_processors);

	double freq_hz;
	while (1) {
		dg_delay_ms(100);
		if (cpu_get_current_frequency_ex(&freq_hz) != DGERR_SUCCESS) {
			printf("cpu_get_current_frequency_ex() failed\n");
			return false;
		}
		printf("%lf speed in Hz\n", freq_hz);
		printf("%lf speed in GHz\n", freq_hz / 1e9);
	}
	return true;
}

int DG_DTCALL dtabs_impl(int v) {
	return v < 0 ? -v : v;
}
int DG_DTCALL dtadd_impl(int a, int b) {
	return a + b;
}

bool test_dispatch_tables()
{
	DT_DECL_BEGIN(zombie_dt)
	DT_DECL_FUNCTION(int, dtabs, int)
	DT_DECL_FUNCTION(int, dtadd, int, int)
	DT_DECL_END();

#define TEST_DT_FAMILY 0
#define TEST_DT_OBJID 1

	DT_INIT_BEGIN(zombie_dt_t, g_test_disptable, TEST_DT_FAMILY, TEST_DT_OBJID)
	DT_INIT_FUNCTION(dtabs, dtabs_impl)
	DT_INIT_FUNCTION(dtadd, dtadd_impl)
	DT_INIT_END()
	return true;
}
bool test_queues()
{
	dg_queue_t queue = queue_init(int, 16);
	if (!queue_alloc(&queue)) {
		printf("queue alloc failed!\n");
		return false;
	}

	return false;
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
	dg_darray3d_t array3d = darray3d_init(10, 10, 10, int);
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
	dg_darray2d_t array2d = darray2d_init(10, 10, int);
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
	dg_darray_t dynarray = darray_init(int, 10, 10, 0);
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
bool test_handles()
{
	dg_handle_alloc_t allocator;
	if (!ha_init(&allocator, sizeof(int), 1024, 1)) {
		printf("failed to initialize handle allocator\n");
		return false;
	}

	/* alloc handle */
	handle_t           opened_handle;
	dg_halloc_result_t allocated;
	if (!ha_alloc_handle(&allocated, &allocator)) {
		printf("ha_alloc_handle() failed\n");
		return false;
	}
	opened_handle = allocated.new_handle;
	*((int*)allocated.phandle_body) = 1000;
	ha_free_handle(&allocator, opened_handle);

	dg_darray_t opened_handles = darray_init(handle_t, 1024, 1024, 0);
	for (size_t i = 0; i < 512; i++) {
		if (ha_alloc_handle(&allocated, &allocator)) {
			//printf("allocated handle: %");
			handle_t handle = allocated.new_handle;
			if (!darray_push_back(&opened_handles, &handle)) {
				printf("darray_push_back() failed!");
				return false;
			}
		}
		else {
			printf("handle allocation failed!\n");
			return false;
		}
	}

	printf("=============== list handles ===============\n");
	for (size_t i = 0; i < darray_get_size(&opened_handles); i++) {
		handle_t value = darray_get(&opened_handles, i, handle_t);
		printf("[handle %zd] idx: %u  gen: %u\n", i, value.index, value.gen);
		if (!ha_free_handle(&allocator, value)) {
			printf("\n");
			return false;
		}
	}
	printf("\n");

	printf("test double-free handle: ");
	if (ha_free_handle(&allocator, opened_handle)) {
		printf("double free handle completed with no errors! it's abnormal!\n");
		return false;
	}
	else {
		printf("OK!\n");
	}

	darray_free(&opened_handles);
	ha_deinit(&allocator);
	return true;
}
bool test_bitvec()
{
	size_t i;
	dg_dbitvec_t bitvec;
	enum { TEST_BITVEC_SIZE=256 };
	if (!dbitvec_init(&bitvec, TEST_BITVEC_SIZE)) {
		printf("dbitvec_init() failed\n");
		return false;
	}

	for (i = 0; i < TEST_BITVEC_SIZE; i++)
		dbitvec_set(&bitvec, i, (int)(i & 1u));

	printf("----------- bitvec test ----------\n");
	for (i = 0; i < TEST_BITVEC_SIZE; i++)
		printf("   bit %zd value: %d\n", i, (int)dbitvec_get(&bitvec, i));

	dbitvec_deinit(&bitvec);
}

void some_work(uint32_t n)
{
	printf("some_work(): left %d times\n", n);
	if (!n)
		return;

	uint32_t i = 1;
	while (i)
		i++;

	some_work(n-1);
}

int thread_proc(struct dg_thrd_data_s* ptinfo)
{
	printf("=== started dg thread ===\n");

	uint32_t delay_ms = 1000;
	printf("waiting %u ms\n", delay_ms);
	dg_delay_ms(delay_ms);

	printf("=== running some work ===\n");
	some_work(1);

	return 0;
}

void pre_start_thread_proc(struct dg_thrd_data_s* ptinfo)
{
	printf("pre_start_thread_proc() called with info 0x%p\n", ptinfo);
}

void post_thread_proc(struct dg_thrd_data_s* ptinfo)
{
	printf("post_thread_proc() called with info 0x%p\n", ptinfo);
}

bool init_thread_subsystem();

bool test_threads()
{
	int result;
	if (!init_thread_subsystem()) {
		printf("init_thread_subsystem() failed!\n");
		return false;
	}

	dg_thrd_data_t* pthread_data = dg_get_curr_thread_data();
	if (pthread_data) {
		printf("dg_get_curr_thread_data() unexpected behaviour! returned address 0x%p. Must be NULL\n",
			pthread_data);
		return false;
	}

	dg_thread_init_info_t thread_init_info = {
		.affinity=DGT_AUTO_AFFINITY,
		.flags=DGTF_USE_LINALLOC,
		.linalloc_size=1024,
		.priority=DGPRIOR_LOW,
		.pthread_end_routine = post_thread_proc,
		.pthread_pre_routine = pre_start_thread_proc,
		.pthread_start_routine = thread_proc,
		.puserptr=(void*)1000,
		.stack_size=0
	};

	dg_thrd_t hthread = dg_thread_create_ex(&thread_init_info);
	if (!hthread) {
		printf("dg_thread_create_ex() failed!\n");
		return false;
	}

	result = dg_thread_join_timed(hthread, 500);
	if (result != DGERR_TIMEOUT) {
		printf("dg_thread_join_timed() must be returned %d but returned %d\n", DGERR_TIMEOUT, result);
		return false;
	}

	result = dg_thread_join(hthread);
	if (result != DGERR_SUCCESS) {
		printf("dg_thread_join() must be return status %d but returned %d\n", DGERR_SUCCESS, result);
		return false;
	}

	/* alloc information for current system thread */
	dg_thrd_data_t* pnewattacheddata = dg_thread_attach_info(DGTF_NONE, 1024);
	if (!pnewattacheddata) {
		printf("dg_thread_attach_info() returned NULL\n");
		return false;
	}

	dg_thrd_data_t *pcurrthreaddata = dg_get_curr_thread_data();
	if (pnewattacheddata != pcurrthreaddata) {
		printf("dg_get_curr_thread_data() returned different information block! Returned 0x%p  must be: 0x%p\n",
			pcurrthreaddata, pnewattacheddata);
		return false;
	}
	return true;
}

void task_start_proc(struct dg_task_s* ptask)
{
	printf("task_start_proc() called! number: %zd\n", (size_t)ptask->puserdata);
	some_work(1);
}

void task_skip_proc(struct dg_task_s* ptask, int taskterm_reason)
{
	printf("task_skip_proc() called! Termination reason: %d\n", taskterm_reason);
}

bool test_threadpool()
{
	int             status;
	dg_threadpool_t threadpool;

	if (!init_thread_subsystem()) {
		printf("init_thread_subsystem() failed!\n");
		return false;
	}

	status = tp_init(&threadpool, 16);
	if (status != DGERR_SUCCESS) {
		printf("threadpool_init() failed with status %d\n", status);
		return false;
	}

	for (size_t i = 0; i < 16; i++) {
		printf("adding task...\n");
		//Sleep(100);
		//getchar();
		status = tp_task_add(&threadpool, task_start_proc, task_skip_proc, DGTASKPRIOR_MIDDLE, (void*)i, 0.);
		if (status != DGERR_SUCCESS) {
			printf("tp_task_add() failed with status %d\n", status);
			continue;
		}
	}
	
	printf("wait for finishing...\n");
	getchar();
	tp_deinit(&threadpool);
	return true;
}
bool test_strings()
{
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
	//RUN_TEST(test_darray1d, "1D array testing failed!")
	//RUN_TEST(test_darray2d, "2D array testing failed!")
	//RUN_TEST(test_darray3d, "3D array testing failed!")
	//RUN_TEST(test_list, "list testing failed!")
	//RUN_TEST(test_queues, "queues testing failed!")
	//RUN_TEST(test_cpuinfo, "cpuinfo testing failed!")
	//RUN_TEST(test_handles, "cpuinfo testing failed!")
	//RUN_TEST(test_bitvec, "bitvec testing failed!")
	//RUN_TEST(test_threads, "threads testing failed!")
	//RUN_TEST(test_threadpool, "threads pool testing failed!")
	RUN_TEST(test_strings, "string testing failed!")



	return 0;
}