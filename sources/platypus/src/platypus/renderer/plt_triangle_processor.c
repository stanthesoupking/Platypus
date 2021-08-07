#include "plt_triangle_processor.h"

#include <stdlib.h>

typedef struct Plt_Triangle_Processor_Working_Buffer {
	unsigned int triangle_capacity;

	int *bounds_min_x;
	int *bounds_min_y;
	int *bounds_max_x;
	int *bounds_max_y;

	simd_int4 *c_initial;
	simd_int4 *c_increment_x;
	simd_int4 *c_increment_y;
} Plt_Triangle_Processor_Working_Buffer;

typedef struct Plt_Triangle_Processor {
	Plt_Triangle_Processor_Working_Buffer working_buffer;
} Plt_Triangle_Processor;

void plt_triangle_processor_free_working_buffer(Plt_Triangle_Processor *processor);

Plt_Triangle_Processor *plt_triangle_processor_create() {
	Plt_Triangle_Processor *processor = malloc(sizeof(Plt_Triangle_Processor));

	processor->working_buffer = (Plt_Triangle_Processor_Working_Buffer){
		.triangle_capacity = 0,
		.bounds_min_x = NULL,
		.bounds_min_y = NULL,
		.bounds_max_x = NULL,
		.bounds_max_y = NULL,
		.c_initial = NULL,
		.c_increment_x = NULL,
		.c_increment_y = NULL
	};

	return processor;
}

void plt_triangle_processor_destroy(Plt_Triangle_Processor **processor) {
	plt_triangle_processor_free_working_buffer(*processor);
	free(*processor);
	*processor = NULL;
}

void plt_triangle_processor_free_working_buffer(Plt_Triangle_Processor *processor) {
	if (processor->working_buffer.triangle_capacity == 0) {
		// Nothing to free.
		return;
	}
	free(processor->working_buffer.bounds_max_x);
	free(processor->working_buffer.bounds_max_y);
	free(processor->working_buffer.bounds_min_x);
	free(processor->working_buffer.bounds_min_y);
	free(processor->working_buffer.c_initial);
	free(processor->working_buffer.c_increment_x);
	free(processor->working_buffer.c_increment_y);
	processor->working_buffer.bounds_max_x = NULL;
	processor->working_buffer.bounds_max_y = NULL;
	processor->working_buffer.bounds_min_x = NULL;
	processor->working_buffer.bounds_min_y = NULL;
	processor->working_buffer.c_initial = NULL;
	processor->working_buffer.c_increment_x = NULL;
	processor->working_buffer.c_increment_y = NULL;
}

simd_int4 plt_triangle_processor_orient2d(simd_int4 a_x, simd_int4 a_y, simd_int4 b_x, simd_int4 b_y, simd_int4 c_x, simd_int4 c_y) {
	return simd_int4_subtract(simd_int4_multiply(simd_int4_subtract(b_x, a_x), simd_int4_subtract(c_y, a_y)), simd_int4_multiply(simd_int4_subtract(b_y, a_y), simd_int4_subtract(c_x, a_x)));
}

void plt_triangle_processor_get_c_increment(simd_int4 a_x, simd_int4 a_y, simd_int4 b_x, simd_int4 b_y, simd_int4 *c_increment_x, simd_int4 *c_increment_y) {
	simd_int4 one_v = simd_int4_create_scalar(1);
	simd_int4 zero_v = simd_int4_create_scalar(0);
	simd_int4 zero = plt_triangle_processor_orient2d(a_x, a_y, b_x, b_y, zero_v, zero_v);

	*c_increment_x = simd_int4_subtract(plt_triangle_processor_orient2d(a_x, a_y, b_x, b_y, one_v, zero_v), zero);
	*c_increment_y = simd_int4_subtract(plt_triangle_processor_orient2d(a_x, a_y, b_x, b_y, zero_v, one_v), zero);
}

void plt_triangle_processor_resize_working_buffer(Plt_Triangle_Processor *processor, unsigned int capacity) {
	if (processor->working_buffer.triangle_capacity == capacity) {
		return;
	}

	if (processor->working_buffer.triangle_capacity > 0) {
		free(processor->working_buffer.bounds_max_x);
		free(processor->working_buffer.bounds_max_y);
		free(processor->working_buffer.bounds_min_x);
		free(processor->working_buffer.bounds_min_y);
		free(processor->working_buffer.c_initial);
		free(processor->working_buffer.c_increment_x);
		free(processor->working_buffer.c_increment_y);
	}

	if (capacity > 0) {
		processor->working_buffer.bounds_max_x = malloc(sizeof(int) * capacity);
		processor->working_buffer.bounds_max_y = malloc(sizeof(int) * capacity);
		processor->working_buffer.bounds_min_x = malloc(sizeof(int) * capacity);
		processor->working_buffer.bounds_min_y = malloc(sizeof(int) * capacity);
		processor->working_buffer.c_initial = malloc(sizeof(simd_int4) * capacity);
		processor->working_buffer.c_increment_x = malloc(sizeof(simd_int4) * capacity);
		processor->working_buffer.c_increment_y = malloc(sizeof(simd_int4) * capacity);
	} else {
		processor->working_buffer.bounds_max_x = NULL;
		processor->working_buffer.bounds_max_y = NULL;
		processor->working_buffer.bounds_min_x = NULL;
		processor->working_buffer.bounds_min_y = NULL;
		processor->working_buffer.c_initial = NULL;
		processor->working_buffer.c_increment_x = NULL;
		processor->working_buffer.c_increment_y = NULL;
	}

	processor->working_buffer.triangle_capacity = capacity;
}


Plt_Triangle_Processor_Result plt_triangle_processor_process_vertex_data(Plt_Triangle_Processor *processor, Plt_Vector2i viewport, Plt_Vertex_Processor_Result vertex_data) {
	unsigned int triangle_count = vertex_data.vertex_count / 3;
	
	if (processor->working_buffer.triangle_capacity < triangle_count) {
		plt_triangle_processor_resize_working_buffer(processor, triangle_count);
	}

	// Input
	int *screen_positions_x = vertex_data.screen_positions_x;
	int *screen_positions_y = vertex_data.screen_positions_y;

	// Output
	int *bounds_min_x = processor->working_buffer.bounds_min_x;
	int *bounds_min_y = processor->working_buffer.bounds_min_y;
	int *bounds_max_x = processor->working_buffer.bounds_max_x;
	int *bounds_max_y = processor->working_buffer.bounds_max_y;
	simd_int4 *c_initial = processor->working_buffer.c_initial;
	simd_int4 *c_increment_x = processor->working_buffer.c_increment_x;
	simd_int4 *c_increment_y = processor->working_buffer.c_increment_y;

	for (unsigned int i = 0; i < triangle_count; ++i) {
		unsigned int v = i * 3;

		// Bounding box calculations
		bounds_min_x[i] = screen_positions_x[v];
		bounds_min_y[i] = screen_positions_y[v];
		bounds_max_x[i] = screen_positions_x[v];
		bounds_max_y[i] = screen_positions_y[v];
		for (unsigned int j = 1; j < 3; ++j) {
			bounds_min_x[i] = plt_min(screen_positions_x[v + j], bounds_min_x[i]);
			bounds_min_y[i] = plt_min(screen_positions_y[v + j], bounds_min_y[i]);
			bounds_max_x[i] = plt_max(screen_positions_x[v + j], bounds_max_x[i]);
			bounds_max_y[i] = plt_max(screen_positions_y[v + j], bounds_max_y[i]);
		}
		
		bounds_min_x[i] = plt_clamp(bounds_min_x[i], 0, viewport.x);
		bounds_min_y[i] = plt_clamp(bounds_min_y[i], 0, viewport.y);
		bounds_max_x[i] = plt_clamp(bounds_max_x[i], 0, viewport.x);
		bounds_max_y[i] = plt_clamp(bounds_max_y[i], 0, viewport.y);

		simd_int4 a_x = simd_int4_create(screen_positions_x[v + 1], screen_positions_x[v + 2], screen_positions_x[v + 0], 0);
		simd_int4 a_y = simd_int4_create(screen_positions_y[v + 1], screen_positions_y[v + 2], screen_positions_y[v + 0], 0);
		simd_int4 b_x = simd_int4_create(screen_positions_x[v + 2], screen_positions_x[v + 0], screen_positions_x[v + 1], 0);
		simd_int4 b_y = simd_int4_create(screen_positions_y[v + 2], screen_positions_y[v + 0], screen_positions_y[v + 1], 0);

		// Barycentric calculations
		c_initial[i] = plt_triangle_processor_orient2d(a_x, a_y, b_x, b_y, simd_int4_create_scalar(bounds_min_x[i]), simd_int4_create_scalar(bounds_min_y[i]));
		
		simd_int4 c_increment_x_result, c_increment_y_result;
		plt_triangle_processor_get_c_increment(a_x, a_y, b_x, b_y, &c_increment_x_result, &c_increment_y_result);
		c_increment_x[i] = c_increment_x_result;
		c_increment_y[i] = c_increment_y_result;
	}

	return (Plt_Triangle_Processor_Result) {
		.triangle_count = triangle_count,
		.bounds_min_x = bounds_min_x,
		.bounds_min_y = bounds_min_y,
		.bounds_max_x = bounds_max_x,
		.bounds_max_y = bounds_max_y,
		.c_initial = c_initial,
		.c_increment_x = c_increment_x,
		.c_increment_y = c_increment_y
	};
}
