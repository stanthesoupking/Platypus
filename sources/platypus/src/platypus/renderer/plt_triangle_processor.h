#pragma once

#include "platypus/platypus.h"
#include "platypus/base/simd.h"
#include "plt_vertex_processor.h"

typedef struct Plt_Triangle_Processor Plt_Triangle_Processor;
typedef struct Plt_Triangle_Processor_Result {
	unsigned int triangle_count;

	int *bounds_min_x;
	int *bounds_min_y;
	int *bounds_max_x;
	int *bounds_max_y;

	simd_int4 *c_initial;
	simd_int4 *c_increment_x;
	simd_int4 *c_increment_y;
} Plt_Triangle_Processor_Result;

Plt_Triangle_Processor *plt_triangle_processor_create();
void plt_triangle_processor_destroy(Plt_Triangle_Processor **processor);

Plt_Triangle_Processor_Result plt_triangle_processor_process_vertex_data(Plt_Triangle_Processor *processor, Plt_Vector2i viewport, Plt_Vertex_Processor_Result vertex_data);