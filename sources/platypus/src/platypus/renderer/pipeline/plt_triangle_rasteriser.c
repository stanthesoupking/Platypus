#include "plt_triangle_rasteriser.h"

#include "platypus/base/thread/plt_thread.h"
#include "platypus/base/thread/plt_thread_safe_stack.h"
#include "platypus/framebuffer/plt_framebuffer.h"
#include "platypus/base/plt_defines.h"
#include "platypus/base/plt_macros.h"

#include "plt_triangle_bin.h"

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

	Plt_Thread_Pool *thread_pool;

	Plt_Size triangle_bin_dimensions;
	unsigned int triangle_bin_count;
	Plt_Triangle_Bin *triangle_bins;
	Plt_Thread_Safe_Stack *triangle_bin_stack;
	
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
	platform_core_count = 1;
	rasteriser->thread_pool = plt_thread_pool_create(_raster_thread, rasteriser, platform_core_count);
	
	rasteriser->triangle_bins = NULL;
	rasteriser->triangle_bin_count = 0;
	rasteriser->triangle_bin_stack = plt_thread_safe_stack_create(4096);

	return rasteriser;
}

void plt_triangle_rasteriser_destroy(Plt_Triangle_Rasteriser **rasteriser) {
	plt_thread_pool_destroy(&(*rasteriser)->thread_pool);
	free(*rasteriser);
	*rasteriser = NULL;
}

void plt_triangle_rasteriser_update_framebuffer(Plt_Triangle_Rasteriser *rasteriser, Plt_Framebuffer framebuffer) {
	rasteriser->framebuffer = framebuffer;
	rasteriser->viewport_size = (Plt_Size){ framebuffer.width, framebuffer.height };
	
	rasteriser->triangle_bin_dimensions = plt_size_make(rasteriser->viewport_size.width / PLT_TRIANGLE_BIN_SIZE, rasteriser->viewport_size.height / PLT_TRIANGLE_BIN_SIZE);
	
	unsigned int required_triangle_bin_count = rasteriser->triangle_bin_dimensions.width * rasteriser->triangle_bin_dimensions.height;
	if (rasteriser->triangle_bin_count < required_triangle_bin_count) {
		if (rasteriser->triangle_bins) {
			free(rasteriser->triangle_bins);
		}
		
		if (rasteriser->triangle_bin_stack->capacity < required_triangle_bin_count) {
			plt_thread_safe_stack_destroy(&rasteriser->triangle_bin_stack);
			rasteriser->triangle_bin_stack = plt_thread_safe_stack_create(required_triangle_bin_count + 1);
		}
		
		rasteriser->triangle_bins = malloc(sizeof(Plt_Triangle_Bin) * required_triangle_bin_count);
	}
	rasteriser->triangle_bin_count = required_triangle_bin_count;
}

void plt_triangle_rasteriser_update_depth_buffer(Plt_Triangle_Rasteriser *rasteriser, float *depth_buffer) {
	rasteriser->depth_buffer = depth_buffer;
}

#define RENDER_PIXEL \
if ((bc_x.x <= 0) && (bc_x.y <= 0) && (bc_x.z <= 0) && ((bc_x.x | bc_x.y | bc_x.z) != 0)) { \
	float sum = bc_x.x + bc_x.y + bc_x.z; \
	simd_float4 weights = simd_float4_create(bc_x.x / sum, bc_x.y / sum, bc_x.z / sum, 0.0f); \
	*px = plt_color8_make(weights.x * 255, weights.y * 255, weights.z * 255, 255); \
} \
px++; \
bc_x = simd_int4_add(bc_x, bc_increment_x); \

void *_raster_thread(unsigned int thread_id, void *thread_data) {
	Plt_Triangle_Rasteriser *rasteriser = thread_data;
	Plt_Renderer *renderer = rasteriser->renderer;
	
	Plt_Color8 *pixels = rasteriser->framebuffer.pixels;
	float *depth_buffer = rasteriser->depth_buffer;
	Plt_Size viewport_size = rasteriser->viewport_size;
	
	Plt_Color8 clear_color = plt_color8_make(100, 120, 160, 255);
	
	const Plt_Color8 debug_colors[6] = {
		plt_color8_make(0, 255, 0, 255),
		plt_color8_make(0, 0, 255, 255),
		plt_color8_make(255, 0, 0, 255),
		plt_color8_make(255, 255, 0, 255),
		plt_color8_make(255, 0, 255, 255),
		plt_color8_make(0, 255, 255, 255),
	};
	Plt_Color8 debug_color = debug_colors[thread_id % 6];
	
	unsigned int bin_index;
	while (plt_thread_safe_stack_pop(rasteriser->triangle_bin_stack, &bin_index)) {
		Plt_Triangle_Bin bin = rasteriser->triangle_bins[bin_index];
		
		// Render triangle bin
		Plt_Rect bin_region = plt_rect_make((bin_index % rasteriser->triangle_bin_dimensions.width) * PLT_TRIANGLE_BIN_SIZE, (bin_index / rasteriser->triangle_bin_dimensions.width) * PLT_TRIANGLE_BIN_SIZE, PLT_TRIANGLE_BIN_SIZE, PLT_TRIANGLE_BIN_SIZE);
		
		Plt_Color8 *pixel_initial = pixels + bin_region.y * viewport_size.width + bin_region.x;
		float *depth_initial = depth_buffer + bin_region.y * viewport_size.width + bin_region.x;
		
		// Step 1: Clear tile with color
		{
			Plt_Color8 *py = pixel_initial;
			float *dy = depth_initial;
			for (unsigned int y = 0; y < PLT_TRIANGLE_BIN_SIZE; ++y) {
				Plt_Color8 *px = py;
				float *dx = dy;
				for (unsigned int x = 0; x < PLT_TRIANGLE_BIN_SIZE; ++x) {
					*(px++) = clear_color;
					*(dx++) = 0.0f;
				}
				py += viewport_size.width;
				dy += viewport_size.width;
			}
		}
		
		// Step 2: Rasterise triangles in bin
		{
			for (unsigned int i = 0; i < bin.triangle_count; ++i) {
				Plt_Triangle_Bin_Entry entry = bin.entries[i];
				Plt_Triangle_Bin_Data_Buffer *data_buffer = entry.buffer;
				
				Plt_Texture *texture = entry.texture;
				Plt_Size texture_size = plt_texture_get_size(texture);
				Plt_Color8 *texture_pixels = plt_texture_get_pixels(texture);
				
				simd_int4 bc_initial = data_buffer->bc_initial[entry.index];
				simd_int4 bc_increment_x = data_buffer->bc_increment_x[entry.index];
				simd_int4 bc_increment_y = data_buffer->bc_increment_y[entry.index];
				float triangle_area = data_buffer->triangle_area[entry.index];
				
				float depth0 = data_buffer->depth0[entry.index];
				float depth1 = data_buffer->depth1[entry.index];
				float depth2 = data_buffer->depth2[entry.index];
				
				Plt_Vector2f uv0 = data_buffer->uv0[entry.index];
				Plt_Vector2f uv1 = data_buffer->uv1[entry.index];
				Plt_Vector2f uv2 = data_buffer->uv2[entry.index];
				
				simd_int4 bc_y = simd_int4_add(simd_int4_add(bc_initial, simd_int4_multiply(bc_increment_y, simd_int4_create_scalar(bin_region.y))), simd_int4_multiply(bc_increment_x, simd_int4_create_scalar(bin_region.x)));
				Plt_Color8 *py = pixel_initial;
				float *dy = depth_initial;
				for (unsigned int y = 0; y < PLT_TRIANGLE_BIN_SIZE; ++y) {
					simd_int4 bc_x = bc_y;
					Plt_Color8 *px = py;
					float *dx = dy;
					for (unsigned int x = 0; x < PLT_TRIANGLE_BIN_SIZE; ++x) {
						if ((bc_x.x <= 0) && (bc_x.y <= 0) && (bc_x.z <= 0) && ((bc_x.x | bc_x.y | bc_x.z) != 0)) {
							simd_float4 weights = simd_float4_create(bc_x.x / triangle_area, bc_x.y / triangle_area, bc_x.z / triangle_area, 0.0f);
							
							float depth = depth0 * weights.x + depth1 * weights.y + depth2 * weights.z;
							if (depth > *dx) {
								*dx = depth;
								Plt_Vector2i tex_coord = {
									.x = (uv0.x * weights.x + uv1.x * weights.y + uv2.x * weights.z) * texture_size.width,
									.y = (uv0.y * weights.x + uv1.y * weights.y + uv2.y * weights.z) * texture_size.height
								};
								#if PLT_DEBUG_RASTER_THREAD_ID
								*px = debug_color;
								#else
								*px = plt_texture_get_pixel(texture, tex_coord);
//								*px = plt_color8_make(depth * 255, depth * 255, depth * 255, 255);
								#endif
							}
						}
						++px;
						++dx;
						bc_x = simd_int4_add(bc_x, bc_increment_x);
					}
					py += viewport_size.width;
					dy += viewport_size.width;
					bc_y = simd_int4_add(bc_y, bc_increment_y);
				}
			}
		}
		
	}
	return NULL;
}

void plt_triangle_rasteriser_render_triangles(Plt_Triangle_Rasteriser *rasteriser) {	
	// Add all bins to be processed
	rasteriser->triangle_bin_stack->count = 0;
	for (unsigned int i = 0; i < rasteriser->triangle_bin_count; ++i) {
		plt_thread_safe_stack_push(rasteriser->triangle_bin_stack, i);
	}
	
	plt_thread_pool_signal_data_ready(rasteriser->thread_pool);
	plt_thread_pool_wait_until_complete(rasteriser->thread_pool);
}

Plt_Size plt_rasteriser_get_triangle_bin_dimensions(Plt_Triangle_Rasteriser *rasteriser) {
	return rasteriser->triangle_bin_dimensions;
}

Plt_Triangle_Bin *plt_rasteriser_get_triangle_bin(Plt_Triangle_Rasteriser *rasteriser, Plt_Vector2i position) {
	position.x = plt_clamp(position.x, 0, rasteriser->triangle_bin_dimensions.width - 1);
	position.y = plt_clamp(position.y, 0, rasteriser->triangle_bin_dimensions.height - 1);
	return &rasteriser->triangle_bins[position.y * rasteriser->triangle_bin_dimensions.width + position.x];
}

void plt_rasteriser_clear_triangle_bins(Plt_Triangle_Rasteriser *rasteriser) {
	rasteriser->triangle_bin_stack->count = 0;
	for (unsigned int i = 0; i < rasteriser->triangle_bin_count; ++i) {
		rasteriser->triangle_bins[i].triangle_count = 0;
	}
}
