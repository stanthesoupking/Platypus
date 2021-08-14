#include "plt_linear_allocator.h"

#include <stdlib.h>
#include "platypus/base/plt_macros.h"

typedef struct Plt_Linear_Allocator {
	void *heap;
	unsigned int heap_ptr;

	unsigned int capacity; 
} Plt_Linear_Allocator;

Plt_Linear_Allocator *plt_linear_allocator_create(size_t capacity) {
	Plt_Linear_Allocator *allocator = malloc(sizeof(Plt_Linear_Allocator));

	allocator->heap = malloc(capacity);
	allocator->heap_ptr = 0;

	allocator->capacity = capacity;

	return allocator;
}

void plt_linear_allocator_destroy(Plt_Linear_Allocator **allocator) {
	free((*allocator)->heap);
	free(*allocator);
	*allocator = NULL;
}

void *plt_linear_allocator_alloc(Plt_Linear_Allocator *allocator, size_t length) {
	plt_assert(allocator->heap_ptr + length < allocator->capacity, "Linear allocator exhausted.");
	void *allocation = ((char *)allocator->heap) + allocator->heap_ptr;
	allocator->heap_ptr += length;
	return allocation;
}

void plt_linear_allocator_clear(Plt_Linear_Allocator *allocator) {
	allocator->heap_ptr = 0;
}
