#include "plt_triangle_rasteriser.h"

#include "platypus/base/thread/plt_thread.h"
#include "platypus/framebuffer/plt_framebuffer.h"
#include "platypus/base/plt_defines.h"
#include "platypus/base/plt_macros.h"

#include <math.h>

void *_raster_thread(unsigned int thread_id, void *thread_data);
typedef struct Plt_Triangle_Rasteriser_Thread_Data {
	unsigned int thread_id;
	Plt_Rect region;
	Plt_Triangle_Rasteriser *rasteriser;
	Plt_Triangle_Processor_Result tp_result;
	Plt_Vertex_Processor_Result vp_result;
} Plt_Triangle_Rasteriser_Thread_Data;

typedef struct Plt_Triangle_Rasteriser {
	Plt_Renderer *renderer;
	Plt_Size viewport_size;
	Plt_Framebuffer framebuffer;
	float *depth_buffer;

	unsigned int thread_count;
	Plt_Size thread_dimensions;

	Plt_Thread_Pool *thread_pool;
	
	Plt_Rect *thread_regions;
	Plt_Vertex_Processor_Result thread_vp_result;
	Plt_Triangle_Processor_Result thread_tp_result;
} Plt_Triangle_Rasteriser;

Plt_Triangle_Rasteriser *plt_triangle_rasteriser_create(Plt_Renderer *renderer, Plt_Size viewport_size) {
	Plt_Triangle_Rasteriser *rasteriser = malloc(sizeof(Plt_Triangle_Rasteriser));

	rasteriser->renderer = renderer;
	rasteriser->viewport_size = viewport_size;
	rasteriser->framebuffer = (Plt_Framebuffer){
		.pixels = NULL,
		.width = 0,
		.height = 0
	};
	rasteriser->depth_buffer = NULL;

	// Create threads
	unsigned int platform_core_count = plt_platform_get_core_count();
	plt_assert(platform_core_count > 0, "No cores detected on device\n");

	float dim = ceilf(sqrtf(platform_core_count));
	rasteriser->thread_dimensions = (Plt_Size){ dim, dim };
	rasteriser->thread_count = rasteriser->thread_dimensions.width * rasteriser->thread_dimensions.height;

	rasteriser->thread_regions = malloc(sizeof(Plt_Rect) * rasteriser->thread_count);
	rasteriser->thread_pool = plt_thread_pool_create(_raster_thread, rasteriser, rasteriser->thread_count);

	return rasteriser;
}

void plt_triangle_rasteriser_destroy(Plt_Triangle_Rasteriser **rasteriser) {
	plt_thread_pool_destroy(&(*rasteriser)->thread_pool);
	free(&(*rasteriser)->thread_regions);
	free(*rasteriser);
	*rasteriser = NULL;
}

void plt_triangle_rasteriser_update_framebuffer(Plt_Triangle_Rasteriser *rasteriser, Plt_Framebuffer framebuffer) {
	rasteriser->framebuffer = framebuffer;
	rasteriser->viewport_size = (Plt_Size){ framebuffer.width, framebuffer.height };

	// Update thread regions
	Plt_Size thread_region_size = {
		.width = rasteriser->viewport_size.width / rasteriser->thread_dimensions.width,
		.height = rasteriser->viewport_size.height / rasteriser->thread_dimensions.height
	};
	
	Plt_Size thread_region_size_remainder = {
		.width = thread_region_size.width + rasteriser->viewport_size.width % rasteriser->thread_dimensions.width,
		.height = thread_region_size.height + rasteriser->viewport_size.height % rasteriser->thread_dimensions.height
	};
	
	for (unsigned int y = 0; y < rasteriser->thread_dimensions.height; ++y) {
		for (unsigned int x = 0; x < rasteriser->thread_dimensions.width; ++x) {
			unsigned int index = y * rasteriser->thread_dimensions.width + x;
			rasteriser->thread_regions[index] = (Plt_Rect) {
				.x = x * thread_region_size.width,
				.y = y * thread_region_size.height,
				.width = (x == rasteriser->thread_dimensions.width - 1) ? thread_region_size_remainder.width : thread_region_size.width,
				.height = (y == rasteriser->thread_dimensions.height - 1) ? thread_region_size_remainder.height : thread_region_size.height
			};
		}
	}
}

void plt_triangle_rasteriser_update_depth_buffer(Plt_Triangle_Rasteriser *rasteriser, float *depth_buffer) {
	rasteriser->depth_buffer = depth_buffer;
}

// Define rasterisation functions:
#define RASTER_FUNC_NAME _draw_triangle_textured_unlit
#define RASTER_TEXTURED 1
#define RASTER_LIGHTING_MODEL 0
#include "plt_raster_function.h"

#define RASTER_FUNC_NAME _draw_triangle_textured_lit
#define RASTER_TEXTURED 1
#define RASTER_LIGHTING_MODEL 1
#include "plt_raster_function.h"

#define RASTER_FUNC_NAME _draw_triangle_untextured_unlit
#define RASTER_TEXTURED 0
#define RASTER_LIGHTING_MODEL 0
#include "plt_raster_function.h"

#define RASTER_FUNC_NAME _draw_triangle_untextured_lit
#define RASTER_TEXTURED 0
#define RASTER_LIGHTING_MODEL 1
#include "plt_raster_function.h"

#define RASTER_FUNC_NAME _draw_triangle_debug_thread_id
#define RASTER_TEXTURED 0
#define RASTER_LIGHTING_MODEL 1
#define RASTER_DEBUG_THREAD_ID 1
#include "plt_raster_function.h"

void *_raster_thread(unsigned int thread_id, void *thread_data) {
	Plt_Triangle_Rasteriser *rasteriser = thread_data;
	Plt_Renderer *renderer = rasteriser->renderer;
	
	Plt_Rect region = rasteriser->thread_regions[thread_id];

	#if PLT_DEBUG_RASTER_THREAD_ID
	_draw_triangle_debug_thread_id(rasteriser, region, thread_id, rasteriser->thread_vp_result, rasteriser->thread_tp_result);
	return NULL;
	#endif
	
	if (renderer->bound_texture) {
		if (renderer->lighting_model == Plt_Lighting_Model_Unlit) {
			_draw_triangle_textured_unlit(rasteriser, region, thread_id, rasteriser->thread_vp_result, rasteriser->thread_tp_result);
		} else {
			_draw_triangle_textured_lit(rasteriser, region, thread_id, rasteriser->thread_vp_result, rasteriser->thread_tp_result);
		}
	} else {
		if (renderer->lighting_model == Plt_Lighting_Model_Unlit) {
			_draw_triangle_untextured_unlit(rasteriser, region, thread_id, rasteriser->thread_vp_result, rasteriser->thread_tp_result);
		} else {
			_draw_triangle_untextured_lit(rasteriser, region, thread_id, rasteriser->thread_vp_result, rasteriser->thread_tp_result);
		}
	}

	return NULL;
}

void plt_triangle_rasteriser_render_triangles(Plt_Triangle_Rasteriser *rasteriser, Plt_Triangle_Processor_Result tp_result, Plt_Vertex_Processor_Result vp_result) {
	rasteriser->thread_tp_result = tp_result;
	rasteriser->thread_vp_result = vp_result;
	
	plt_thread_pool_signal_data_ready(rasteriser->thread_pool);
	plt_thread_pool_wait_until_complete(rasteriser->thread_pool);
}
