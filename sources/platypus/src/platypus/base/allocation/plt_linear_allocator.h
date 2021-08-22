#pragma once

#include "platypus/platypus.h"
#include <stdlib.h>

typedef struct Plt_Linear_Allocator Plt_Linear_Allocator;

Plt_Linear_Allocator *plt_linear_allocator_create(size_t capacity);
void plt_linear_allocator_destroy(Plt_Linear_Allocator **allocator);

void *plt_linear_allocator_alloc(Plt_Linear_Allocator *allocator, size_t length);
void plt_linear_allocator_clear(Plt_Linear_Allocator *allocator);
