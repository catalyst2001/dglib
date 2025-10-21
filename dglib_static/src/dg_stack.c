#include "dg_stack.h"

void stack_init(dg_stack_t* stack, size_t elem_size, size_t initial_capacity)
{
	if (!stack || elem_size == 0 || initial_capacity == 0)
		return;
	stack->data = (uint8_t*)malloc(elem_size * initial_capacity);
	if (!stack->data) {
		stack->elem_size = 0;
		stack->size = 0;
		stack->capacity = 0;
		stack->tail = 0;
		stack->head = 0;
		return;
	}
	stack->elem_size = elem_size;
	stack->size = 0;
	stack->capacity = initial_capacity;
	stack->tail = 0;
	stack->head = 0;
}

void stack_push_back(dg_stack_t* stack, const void* elem)
{
	if (!stack || !stack->data || !elem)
		return;
	// Resize if needed
	if (stack->size >= stack->capacity) {
		size_t new_capacity = stack->capacity * 2;
		uint8_t* new_data = (uint8_t*)realloc(stack->data, stack->elem_size * new_capacity);
		if (!new_data)
			return; // realloc failed, keep old data
		stack->data = new_data;
		stack->capacity = new_capacity;
	}
	// Add element at tail
	size_t index = (stack->head + stack->size) % stack->capacity;
	memcpy(&stack->data[index * stack->elem_size], elem, stack->elem_size);
	stack->size++;
}

void stack_push_front(dg_stack_t* stack, const void* elem)
{
	if (!stack || !stack->data || !elem)
		return;
	// Resize if needed
	if (stack->size >= stack->capacity) {
		size_t new_capacity = stack->capacity * 2;
		uint8_t* new_data = (uint8_t*)realloc(stack->data, stack->elem_size * new_capacity);
		if (!new_data)
			return; // realloc failed, keep old data
		stack->data = new_data;
		stack->capacity = new_capacity;
	}
	// Add element at head
	stack->head = (stack->head == 0) ? (stack->capacity - 1) : (stack->head - 1);
	memcpy(&stack->data[stack->head * stack->elem_size], elem, stack->elem_size);
	stack->size++;
}

void stack_pop_back(dg_stack_t* stack, void* elem)
{
	if (!stack || !stack->data || stack->size == 0)
		return;
	// Remove element from tail
	size_t index = (stack->head + stack->size - 1) % stack->capacity;
	if (elem) {
		memcpy(elem, &stack->data[index * stack->elem_size], stack->elem_size);
	}
	stack->size--;
}

void stack_pop_front(dg_stack_t* stack, void* elem)
{
	if (!stack || !stack->data || stack->size == 0)
		return;
	// Remove element from head
	if (elem) {
		memcpy(elem, &stack->data[stack->head * stack->elem_size], stack->elem_size);
	}
	stack->head = (stack->head + 1) % stack->capacity;
	stack->size--;
}

void stack_peek(const dg_stack_t* stack, void* elem)
{
	if (!stack || !stack->data || stack->size == 0 || !elem)
		return;
	// Peek at the top element (tail)
	size_t index = (stack->head + stack->size - 1) % stack->capacity;
	memcpy(elem, &stack->data[index * stack->elem_size], stack->elem_size);
}

void stack_clear(dg_stack_t* stack)
{
	if (!stack || !stack->data)
		return;
	stack->size = 0;
	stack->head = 0;
	stack->tail = 0;
}

void stack_free(dg_stack_t* stack)
{
	if (!stack)
		return;
	if (stack->data) {
		free(stack->data);
		stack->data = NULL;
	}
	stack->elem_size = 0;
	stack->size = 0;
	stack->capacity = 0;
	stack->head = 0;
	stack->tail = 0;
}

bool stack_is_empty(const dg_stack_t* stack)
{
	if (!stack)
		return true;

	return (stack->size == 0);
}
