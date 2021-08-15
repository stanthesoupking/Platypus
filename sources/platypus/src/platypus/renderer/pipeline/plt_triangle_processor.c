#include "plt_triangle_processor.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "platypus/base/allocation/plt_linear_allocator.h"
#include "plt_triangle_rasteriser.h"
#include "plt_triangle_bin.h"

typedef struct Plt_Triangle_Processor {
	bool idk;
} Plt_Triangle_Processor;

void plt_triangle_processor_free_working_buffer(Plt_Triangle_Processor *processor);

Plt_Triangle_Processor *plt_triangle_processor_create() {
	Plt_Triangle_Processor *processor = malloc(sizeof(Plt_Triangle_Processor));
	return processor;
}

void plt_triangle_processor_destroy(Plt_Triangle_Processor **processor) {
	free(*processor);
	*processor = NULL;
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

Plt_Vector2f plt_triangle_processor_get_texture_coordinate(simd_int4 bc, Plt_Size texture_size, float *uv_x, float *uv_y) {
	Plt_Vector2f result;
	
	float initial_sum = bc.x + bc.y + bc.z;
	simd_float4 texture_pixel_weights;
	texture_pixel_weights.x = bc.x / initial_sum;
	texture_pixel_weights.y = bc.y / initial_sum;
	texture_pixel_weights.z = bc.z / initial_sum;
	
	result.x = uv_x[0] * texture_pixel_weights.x + uv_x[1] * texture_pixel_weights.y + uv_x[2] * texture_pixel_weights.z;
	result.y = uv_y[0] * texture_pixel_weights.x + uv_y[1] * texture_pixel_weights.y + uv_y[2] * texture_pixel_weights.z;
	
	return result;
}

// Is the given point (pixel in screenspace) inside the given triangle
bool plt_triangle_processor_is_point_in_triangle(Plt_Vector2i point, simd_int4 bc_initial, simd_int4 bc_increment_x, simd_int4 bc_increment_y) {
	simd_int4 bc = simd_int4_add(simd_int4_add(bc_initial, simd_int4_multiply(bc_increment_y, simd_int4_create_scalar(point.y))), simd_int4_multiply(bc_increment_x, simd_int4_create_scalar(point.x)));
	return (bc.x <= 0) && (bc.y <= 0) && (bc.z <= 0);
}

void plt_triangle_processor_process_vertex_data(Plt_Triangle_Processor *processor, Plt_Linear_Allocator *allocator, Plt_Vector2i viewport, Plt_Texture *texture, Plt_Vertex_Processor_Result vertex_data, Plt_Triangle_Rasteriser *rasteriser) {
	unsigned int triangle_count = vertex_data.vertex_count / 3;
	Plt_Size texture_size = plt_texture_get_size(texture);
	Plt_Vector2f texel_size = plt_texture_get_texel_size(texture);

	// Input
	float *clipspace_z = vertex_data.clipspace_z;
	float *clipspace_w = vertex_data.clipspace_w;
	int *screen_positions_x = vertex_data.screen_positions_x;
	int *screen_positions_y = vertex_data.screen_positions_y;
	float *uv_x = vertex_data.model_uvs_x;
	float *uv_y = vertex_data.model_uvs_y;

	// Output
	Plt_Triangle_Bin_Data_Buffer *data_buffer = plt_linear_allocator_alloc(allocator, sizeof(Plt_Triangle_Bin_Data_Buffer));
	
	// Barycentric coordinates
	simd_int4 *bc_initial = plt_linear_allocator_alloc(allocator, sizeof(simd_int4) * triangle_count);
	simd_int4 *bc_increment_x = plt_linear_allocator_alloc(allocator, sizeof(simd_int4) * triangle_count);
	simd_int4 *bc_increment_y = plt_linear_allocator_alloc(allocator, sizeof(simd_int4) * triangle_count);
	float *triangle_area = plt_linear_allocator_alloc(allocator, sizeof(float) * triangle_count);
	
	// Depth
	float *depth0 = plt_linear_allocator_alloc(allocator, sizeof(float) * triangle_count);
	float *depth1 = plt_linear_allocator_alloc(allocator, sizeof(float) * triangle_count);
	float *depth2 = plt_linear_allocator_alloc(allocator, sizeof(float) * triangle_count);
	
	// Texture coordinates (in pixels)
	Plt_Vector2f *uv0 = plt_linear_allocator_alloc(allocator, sizeof(Plt_Vector2f) * triangle_count);
	Plt_Vector2f *uv1 = plt_linear_allocator_alloc(allocator, sizeof(Plt_Vector2f) * triangle_count);
	Plt_Vector2f *uv2 = plt_linear_allocator_alloc(allocator, sizeof(Plt_Vector2f) * triangle_count);
	
	// Lighting
	Plt_Vector3f *lighting_initial = plt_linear_allocator_alloc(allocator, sizeof(Plt_Vector3f) * triangle_count);
	Plt_Vector3f *lighting_increment_x = plt_linear_allocator_alloc(allocator, sizeof(Plt_Vector3f) * triangle_count);
	Plt_Vector3f *lighting_increment_y = plt_linear_allocator_alloc(allocator, sizeof(Plt_Vector3f) * triangle_count);

	unsigned int output_triangle_count = 0;
	for (unsigned int i = 0; i < triangle_count; ++i) {
		unsigned int o = output_triangle_count;
		unsigned int v = i * 3;

		// Don't render triangles behind the camera
		if ((clipspace_w[v] < 0) || (clipspace_w[v + 1] < 0) || (clipspace_w[v + 2] < 0)) {
			continue;
		}

		// Bounding box calculations
		Plt_Vector2i bounds_min;
		Plt_Vector2i bounds_max;
		bounds_min.x = screen_positions_x[v];
		bounds_min.y = screen_positions_y[v];
		bounds_max.x = screen_positions_x[v];
		bounds_max.y = screen_positions_y[v];
		for (unsigned int j = 1; j < 3; ++j) {
			bounds_min.x = plt_min(screen_positions_x[v + j], bounds_min.x);
			bounds_min.y = plt_min(screen_positions_y[v + j], bounds_min.y);
			bounds_max.x = plt_max(screen_positions_x[v + j], bounds_max.x);
			bounds_max.y = plt_max(screen_positions_y[v + j], bounds_max.y);
		}
		bounds_min.x = plt_clamp(bounds_min.x, 0, viewport.x);
		bounds_min.y = plt_clamp(bounds_min.y, 0, viewport.y);
		bounds_max.x = plt_clamp(bounds_max.x, 0, viewport.x);
		bounds_max.y = plt_clamp(bounds_max.y, 0, viewport.y);

		simd_int4 a_x = simd_int4_create(screen_positions_x[v + 1], screen_positions_x[v + 2], screen_positions_x[v + 0], 0);
		simd_int4 a_y = simd_int4_create(screen_positions_y[v + 1], screen_positions_y[v + 2], screen_positions_y[v + 0], 0);
		simd_int4 b_x = simd_int4_create(screen_positions_x[v + 2], screen_positions_x[v + 0], screen_positions_x[v + 1], 0);
		simd_int4 b_y = simd_int4_create(screen_positions_y[v + 2], screen_positions_y[v + 0], screen_positions_y[v + 1], 0);

		// Cull triangles with zero size
		if ((bounds_max.x - bounds_min.x == 0) || (bounds_max.y - bounds_min.y == 0)) {
			continue;
		}
		
		// Cull backward facing triangles
		Plt_Vector2i center_pixel = { (bounds_max.x + bounds_min.x) / 2, (bounds_max.y + bounds_min.y) / 2 };
		simd_int4 c_center = plt_triangle_processor_orient2d(a_x, a_y, b_x, b_y, simd_int4_create_scalar(center_pixel.x), simd_int4_create_scalar(center_pixel.y));
		bool backward_facing = (c_center.x > 0) && (c_center.y > 0) && (c_center.z > 0);
		if (backward_facing) {
			continue;
		}
		
		// Barycentric calculations
		bc_initial[o] = plt_triangle_processor_orient2d(a_x, a_y, b_x, b_y, simd_int4_create_scalar(0), simd_int4_create_scalar(0));
		triangle_area[o] = bc_initial[o].x + bc_initial[o].y + bc_initial[o].z;
		
		// Degnerate triangle
		if (triangle_area == 0) {
			continue;
		}
		
		simd_int4 bc_increment_x_result, bc_increment_y_result;
		plt_triangle_processor_get_c_increment(a_x, a_y, b_x, b_y, &bc_increment_x_result, &bc_increment_y_result);
		bc_increment_x[o] = bc_increment_x_result;
		bc_increment_y[o] = bc_increment_y_result;
		
		// Texture coordinate calculations
		uv0[o] = plt_vector2f_make(uv_x[v], uv_y[v]);
		uv1[o] = plt_vector2f_make(uv_x[v + 1], uv_y[v + 1]);
		uv2[o] = plt_vector2f_make(uv_x[v + 2], uv_y[v + 2]);
		
		// Depth
		depth0[o] = 1.0f / clipspace_z[v];
		depth1[o] = 1.0f / clipspace_z[v + 1];
		depth2[o] = 1.0f / clipspace_z[v + 2];
		
		Plt_Vector2i tile_bounds_min = plt_vector2i_make(bounds_min.x / PLT_TRIANGLE_BIN_SIZE, bounds_min.y / PLT_TRIANGLE_BIN_SIZE);
		Plt_Vector2i tile_bounds_max = plt_vector2i_make(bounds_max.x / PLT_TRIANGLE_BIN_SIZE, bounds_max.y / PLT_TRIANGLE_BIN_SIZE);
		Plt_Size tile_dimensions = plt_rasteriser_get_triangle_bin_dimensions(rasteriser);
		tile_bounds_max.x = plt_clamp(tile_bounds_max.x, 0, tile_dimensions.width - 1);
		tile_bounds_max.y = plt_clamp(tile_bounds_max.y, 0, tile_dimensions.height - 1);
		for (int y = tile_bounds_min.y; y <= tile_bounds_max.y; ++y) {
			for (int x = tile_bounds_min.x; x <= tile_bounds_max.x; ++x) {
				Plt_Triangle_Bin_Entry bin_entry = {
					.index = o,
					.buffer = data_buffer,
					.texture = texture
				};
								
				Plt_Vector2i top_left = plt_vector2i_make(x * PLT_TRIANGLE_BIN_SIZE, y * PLT_TRIANGLE_BIN_SIZE);
				
				// Get triangle coverage in tile
				bool corner[4];
				corner[0] = plt_triangle_processor_is_point_in_triangle(top_left, bc_initial[o], bc_increment_x[o], bc_increment_y[o]);
				corner[1] = plt_triangle_processor_is_point_in_triangle(plt_vector2i_add(top_left, plt_vector2i_make(0, PLT_TRIANGLE_BIN_SIZE - 1)), bc_initial[o], bc_increment_x[o], bc_increment_y[o]);
				corner[2] = plt_triangle_processor_is_point_in_triangle(plt_vector2i_add(top_left, plt_vector2i_make(PLT_TRIANGLE_BIN_SIZE - 1, PLT_TRIANGLE_BIN_SIZE - 1)), bc_initial[o], bc_increment_x[o], bc_increment_y[o]);
				corner[3] = plt_triangle_processor_is_point_in_triangle(plt_vector2i_add(top_left, plt_vector2i_make(PLT_TRIANGLE_BIN_SIZE - 1, 0)), bc_initial[o], bc_increment_x[o], bc_increment_y[o]);
				
				if (corner[0] & corner[1] & corner[2] & corner[3]) {
					bin_entry.coverage = Plt_Triangle_Tile_Coverage_Full;
				} else {
					bin_entry.coverage = Plt_Triangle_Tile_Coverage_Partial;
				}
								
				Plt_Triangle_Bin *bin = plt_rasteriser_get_triangle_bin(rasteriser, plt_vector2i_make(x, y));
				if (bin->triangle_count < PLT_TRIANGLE_BIN_MAX_TRIANGLES) {
					bin->entries[bin->triangle_count++] = bin_entry;
				}
			}
		}
		
		++output_triangle_count;
	}
	
	data_buffer->triangle_count = output_triangle_count;
	data_buffer->bc_initial = bc_initial;
	data_buffer->bc_increment_x = bc_increment_x;
	data_buffer->bc_increment_y = bc_increment_y;
	data_buffer->triangle_area = triangle_area;
	data_buffer->depth0 = depth0;
	data_buffer->depth1 = depth1;
	data_buffer->depth2 = depth2;
	data_buffer->uv0 = uv0;
	data_buffer->uv1 = uv1;
	data_buffer->uv2 = uv2;
}
