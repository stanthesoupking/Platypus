#include "plt_renderer.h"
#include "plt_raster_function_helpers.h"
#include "platypus/mesh/plt_mesh.h"
#include "platypus/renderer/plt_vertex_processor.h"
#include "platypus/renderer/plt_triangle_processor.h"

#ifndef RASTER_FUNC_NAME
#error "Must supply RASTER_FUNC_NAME"
#endif

#ifndef RASTER_TEXTURED
#error "Must supply RASTER_TEXTURED"
#endif

#ifndef RASTER_LIGHTING_MODEL
#error "Must supply RASTER_LIGHTING_MODEL"
#endif

void RASTER_FUNC_NAME(Plt_Vertex_Processor_Result vertex_data, Plt_Triangle_Processor_Result triangle_data, Plt_Renderer *renderer, Plt_Mesh *mesh) {
	Plt_Color8 *framebuffer_pixels = renderer->framebuffer.pixels;
	float *depth_buffer = renderer->depth_buffer;
	int framebuffer_width = renderer->framebuffer.width;
	int framebuffer_height = renderer->framebuffer.height;
	Plt_Color8 render_color = renderer->render_color;
	
	Plt_Vector3f normalized_light_direction = plt_vector3f_normalize(renderer->directional_lighting_direction);
	
	for (unsigned int i = 0; i < triangle_data.triangle_count; ++i) {
		unsigned int v = i * 3;

		Plt_Vector3f pos[3];
		
		Plt_Vector2i spos[3];
		Plt_Vector4f wpos[3];
		Plt_Vector2f uvs[3];
		Plt_Vector3f normals[3];
		
		#if RASTER_LIGHTING_MODEL == 1
		Plt_Color8 lighting[3];
		#endif
		
		for (unsigned int j = 0; j < 3; ++j) {
			#if RASTER_LIGHTING_MODEL == 1
			normals[j] = (Plt_Vector3f){ vertex_data.world_normals_x[v + j], vertex_data.world_normals_y[v + j], vertex_data.world_normals_z[v + j] };
			
			float light_amount = plt_max(plt_vector3f_dot_product(normals[j], normalized_light_direction), 0);
			Plt_Color8 directional_lighting = plt_color8_multiply_scalar(renderer->directional_lighting_color, light_amount);

			lighting[j] = plt_color8_add(directional_lighting, renderer->ambient_lighting_color);
			#endif
			
			pos[j] = (Plt_Vector3f){ vertex_data.clipspace_x[v + j], vertex_data.clipspace_y[v + j], vertex_data.clipspace_z[v + j] };
			spos[j] = (Plt_Vector2i){ vertex_data.screen_positions_x[v + j], vertex_data.screen_positions_y[v + j] };
			uvs[j] = (Plt_Vector2f){ vertex_data.model_uvs_x[v + j], vertex_data.model_uvs_y[v + j] };
		}

		Plt_Vector2i bounds_min = { triangle_data.bounds_min_x[i], triangle_data.bounds_min_y[i] };
		Plt_Vector2i bounds_max = { triangle_data.bounds_max_x[i], triangle_data.bounds_max_y[i] };

		simd_int4 c_increment_x = triangle_data.c_increment_x[i];
		simd_int4 c_increment_y = triangle_data.c_increment_y[i];
		
		// Half-space triangle rasterization
		simd_int4 cy = triangle_data.c_initial[i];
		for (int y = bounds_min.y; y < bounds_max.y; ++y) {
			simd_int4 cx = cy;
			for (int x = bounds_min.x; x < bounds_max.x; ++x) {
				if ((cx.x <= 0) && (cx.y <= 0) && (cx.z <= 0)) {
					float sum = cx.x + cx.y + cx.z;
					if (sum == 0) {
						goto end;
					}
					
					int framebuffer_pixel_index = y * framebuffer_width + x;
					Plt_Vector3f weights = {cx.x / sum, cx.y / sum, cx.z / sum};

					float depth = 0;
					depth += pos[0].z * weights.x;
					depth += pos[1].z * weights.y;
					depth += pos[2].z * weights.z;

					float depth_sample = depth_buffer[framebuffer_pixel_index];
					if ((depth < 0) || (depth_sample < depth)) {
						goto end;
					}
					depth_buffer[framebuffer_pixel_index] = depth;
					
					#if RASTER_LIGHTING_MODEL == 1
					Plt_Color8 pixel_lighting = {0, 0, 0, 0};
					pixel_lighting = plt_color8_add(pixel_lighting, plt_color8_multiply_scalar(lighting[0], weights.x));
					pixel_lighting = plt_color8_add(pixel_lighting, plt_color8_multiply_scalar(lighting[1], weights.y));
					pixel_lighting = plt_color8_add(pixel_lighting, plt_color8_multiply_scalar(lighting[2], weights.z));
					#endif
					
					Plt_Color8 tex_color;
					#if RASTER_TEXTURED
						Plt_Vector2f uv = {0, 0};
						uv = plt_vector2f_add(uv, plt_vector2f_multiply_scalar(uvs[0], weights.x));
						uv = plt_vector2f_add(uv, plt_vector2f_multiply_scalar(uvs[1], weights.y));
						uv = plt_vector2f_add(uv, plt_vector2f_multiply_scalar(uvs[2], weights.z));
						tex_color = plt_texture_sample(renderer->bound_texture, uv);
					#else
						tex_color = render_color;
					#endif
					
					#if RASTER_LIGHTING_MODEL == 1
						tex_color = plt_color8_multiply(tex_color, pixel_lighting);
					#endif
					
					framebuffer_pixels[framebuffer_pixel_index] = tex_color;
				}
				
				end:
				cx.x += c_increment_x.x; cx.y += c_increment_x.y; cx.z += c_increment_x.z;
			}
			
			cy.x += c_increment_y.x; cy.y += c_increment_y.y; cy.z += c_increment_y.z;
		}
	}
}
