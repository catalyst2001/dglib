#ifndef __dg_stack_h__
#define __dg_stack_h__
#include "dg_libcommon.h"

typedef struct dg_stack_s {
	uint8_t* data;       /*< pointer to data */
	size_t   elem_size;  /*< size of one element */
	size_t   size;       /*< current number of elements in stack */
	size_t   capacity;   /*< current capacity of stack */
	size_t   tail;       /*< index of the top element */
	size_t   head; 		 /*< index of the bottom element */
} dg_stack_t;

void stack_init(dg_stack_t* stack, size_t elem_size, size_t initial_capacity);
void stack_push_back(dg_stack_t* stack, const void* elem);
void stack_push_front(dg_stack_t* stack, const void* elem);
void stack_pop_back(dg_stack_t* stack, void* elem);
void stack_pop_front(dg_stack_t* stack, void* elem);
void stack_peek(const dg_stack_t* stack, void* elem);
void stack_clear(dg_stack_t* stack);
void stack_free(dg_stack_t* stack);
bool stack_is_empty(const dg_stack_t* stack);

#endif // __dg_stack_h__