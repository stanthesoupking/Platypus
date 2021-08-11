#include "plt_triangle_rasteriser.h"

#include "platypus/base/thread/plt_thread.h"
#include "platypus/framebuffer/plt_framebuffer.h"

typedef struct Plt_Triangle_Rasteriser {
	Plt_Renderer *renderer;
	Plt_Size viewport_size;
	Plt_Framebuffer framebuffer;
	float *depth_buffer;
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

	return rasteriser;
}

void plt_triangle_rasteriser_destroy(Plt_Triangle_Rasteriser **rasteriser) {
	free(*rasteriser);
	*rasteriser = NULL;
}

void plt_triangle_rasteriser_update_framebuffer(Plt_Triangle_Rasteriser *rasteriser, Plt_Framebuffer framebuffer) {
	rasteriser->framebuffer = framebuffer;
	rasteriser->viewport_size = (Plt_Size){ framebuffer.width, framebuffer.height };
}

void plt_triangle_rasteriser_update_depth_buffer(Plt_Triangle_Rasteriser *rasteriser, float *depth_buffer) {
	rasteriser->depth_buffer = depth_buffer;
}

// Define rasterisation functions:
#define RASTER_FUNC_NAME _draw_triangle_textured_unlit
#define RASTER_TEXTURED 1
#define RASTER_LIGHTING_MODEL 0
#include "plt_raster_function.h"
#undef RASTER_FUNC_NAME
#undef RASTER_TEXTURED
#undef RASTER_LIGHTING_MODEL

#define RASTER_FUNC_NAME _draw_triangle_textured_lit
#define RASTER_TEXTURED 1
#define RASTER_LIGHTING_MODEL 1
#include "plt_raster_function.h"
#undef RASTER_FUNC_NAME
#undef RASTER_TEXTURED
#undef RASTER_LIGHTING_MODEL

#define RASTER_FUNC_NAME _draw_triangle_untextured_unlit
#define RASTER_TEXTURED 0
#define RASTER_LIGHTING_MODEL 0
#include "plt_raster_function.h"
#undef RASTER_FUNC_NAME
#undef RASTER_TEXTURED
#undef RASTER_LIGHTING_MODEL

#define RASTER_FUNC_NAME _draw_triangle_untextured_lit
#define RASTER_TEXTURED 0
#define RASTER_LIGHTING_MODEL 1
#include "plt_raster_function.h"
#undef RASTER_FUNC_NAME
#undef RASTER_TEXTURED
#undef RASTER_LIGHTING_MODEL

typedef struct Plt_Triangle_Rasteriser_Thread_Data {
	Plt_Rect region;
	Plt_Triangle_Rasteriser *rasteriser;
	Plt_Triangle_Processor_Result tp_result;
	Plt_Vertex_Processor_Result vp_result;
} Plt_Triangle_Rasteriser_Thread_Data;

void *_raster_thread(void *thread_data) {
	Plt_Triangle_Rasteriser_Thread_Data *data = thread_data;
	Plt_Renderer *renderer = data->rasteriser->renderer;

	if (renderer->bound_texture) {
		if (renderer->lighting_model == Plt_Lighting_Model_Unlit) {
			_draw_triangle_textured_unlit(data->rasteriser, data->region, data->vp_result, data->tp_result);
		} else {
			_draw_triangle_textured_lit(data->rasteriser, data->region, data->vp_result, data->tp_result);
		}
	} else {
		if (renderer->lighting_model == Plt_Lighting_Model_Unlit) {
			_draw_triangle_untextured_unlit(data->rasteriser, data->region, data->vp_result, data->tp_result);
		} else {
			_draw_triangle_untextured_lit(data->rasteriser, data->region, data->vp_result, data->tp_result);
		}
	}

	return NULL;
}

void plt_triangle_rasteriser_render_triangles(Plt_Triangle_Rasteriser *rasteriser, Plt_Triangle_Processor_Result tp_result, Plt_Vertex_Processor_Result vp_result) {
	Plt_Size thread_dimensions = { 4, 4 };
	const int total_threads = thread_dimensions.width * thread_dimensions.height;
	
	Plt_Thread **threads = malloc(sizeof(Plt_Thread *) * total_threads);
	Plt_Triangle_Rasteriser_Thread_Data *thread_datas = malloc(sizeof(Plt_Triangle_Rasteriser_Thread_Data) * total_threads);

	Plt_Size thread_region_size = { rasteriser->viewport_size.width / thread_dimensions.width, rasteriser->viewport_size.height / thread_dimensions.height };

	for (unsigned int y = 0; y < thread_dimensions.height; ++y) {
		for (unsigned int x = 0; x < thread_dimensions.width; ++x) {
			unsigned int index = y * thread_dimensions.width + x;

			thread_datas[index].region = (Plt_Rect) {
				.x = x * thread_region_size.width,
				.y = y * thread_region_size.height,
				.width = thread_region_size.width,
				.height = thread_region_size.height
			};
			thread_datas[index].rasteriser = rasteriser;
			thread_datas[index].tp_result = tp_result;
			thread_datas[index].vp_result = vp_result;

			// TODO: Create threads once
			threads[index] = plt_thread_create(_raster_thread, thread_datas + index);
		}
	}

	// Wait for threads to complete
	for (unsigned int i = 0; i < total_threads; ++i) {
		plt_thread_wait_until_complete(threads[i]);
		plt_thread_destroy(&threads[i]);
	}

	free(threads);
	free(thread_datas);
}
