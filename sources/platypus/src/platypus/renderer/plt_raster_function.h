#include "plt_renderer.h"
#include "plt_raster_function_helpers.h"

#ifndef RASTER_FUNC_NAME
#error "Must supply RASTER_FUNC_NAME"
#endif

#ifndef RASTER_TEXTURED
#error "Must supply RASTER_TEXTURED"
#endif

#ifndef RASTER_LIGHTING_MODEL
#error "Must supply RASTER_LIGHTING_MODEL"
#endif

void RASTER_FUNC_NAME(Plt_Renderer *renderer, Plt_Mesh *mesh) {
	Plt_Color8 *framebuffer_pixels = renderer->framebuffer.pixels;
	float *depth_buffer = renderer->depth_buffer;
	int framebuffer_width = renderer->framebuffer.width;
	int framebuffer_height = renderer->framebuffer.height;
	Plt_Color8 render_color = renderer->render_color;
	
	#if RASTER_LIGHTING_MODEL != 0
	Plt_Vector3f light_direction = plt_vector3f_normalize((Plt_Vector3f){-0.75f,-0.25f,0});
	float ambient_light = 0.2f;
	#endif
	
	for (unsigned int i = 0; i < mesh->vertex_count; i += 3) {
		Plt_Vector4f pos[3];
		
		Plt_Vector2i spos[3];
		int32x4_t spos_x[3];
		int32x4_t spos_y[3];
		
		Plt_Vector4f wpos[3];
		Plt_Vector2f uvs[3];
		Plt_Vector3f normals[3];
		Plt_Vector3f face_normal = {};
		
		#if RASTER_LIGHTING_MODEL == 1
		float lighting[3];
		#endif
		
		bool visible = false;
		for (unsigned int j = 0; j < 3; ++j) {
			pos[j] = (Plt_Vector4f) {
				mesh->position_x[i + j],
				mesh->position_y[i + j],
				mesh->position_z[i + j],
				1.0f
			};
			
			#if RASTER_TEXTURED
				uvs[j] = plt_mesh_get_uv(mesh, i + j);
			#endif
			
			#if RASTER_LIGHTING_MODEL == 1
			Plt_Vector3f model_normal = plt_mesh_get_normal(mesh, i + j);
			Plt_Vector4f world_normal = plt_matrix_multiply_vector4f(renderer->model_matrix, (Plt_Vector4f){model_normal.x, model_normal.y, model_normal.z, 0.0f});
			normals[j] = plt_vector3f_normalize((Plt_Vector3f){world_normal.x, world_normal.y, world_normal.z});
			
			face_normal = plt_vector3f_add(face_normal, normals[j]);

			float light_amount = plt_max(plt_vector3f_dot_product(normals[j], light_direction), 0);
			lighting[j] = plt_clamp(light_amount + ambient_light, 0.0f, 1.0f);
			#endif
			
			pos[j] = plt_matrix_multiply_vector4f(renderer->mvp_matrix, pos[j]);
			wpos[j] = (Plt_Vector4f){ pos[j].x / pos[j].w, pos[j].y / pos[j].w, pos[j].z / pos[j].w, 1.0f };
			
			Plt_Vector2i spos_v = plt_renderer_clipspace_to_pixel(renderer, (Plt_Vector2f){wpos[j].x, wpos[j].y});
			spos_x[j] = (int32x4_t){spos_v.x, spos_v.x, spos_v.x, spos_v.x};
			spos_y[j] = (int32x4_t){spos_v.y, spos_v.y, spos_v.y, spos_v.y};
			spos[j] = spos_v;
		}
		
		Plt_Vector2i bounds_min = {spos_x[0][0], spos_y[0][0]};
		Plt_Vector2i bounds_max = {spos_x[0][0], spos_y[0][0]};
		for (unsigned int j = 1; j < 3; ++j) {
			bounds_min.x = plt_min(spos_x[j][0], bounds_min.x);
			bounds_min.y = plt_min(spos_y[j][0], bounds_min.y);
			bounds_max.x = plt_max(spos_x[j][0], bounds_max.x);
			bounds_max.y = plt_max(spos_y[j][0], bounds_max.y);
		}
		
		bounds_min.x = plt_clamp(bounds_min.x, 0, framebuffer_width);
		bounds_min.y = plt_clamp(bounds_min.y, 0, framebuffer_height);
		bounds_max.x = plt_clamp(bounds_max.x, 0, framebuffer_width);
		bounds_max.y = plt_clamp(bounds_max.y, 0, framebuffer_height);
		
		Plt_Vector2i c1_inc = plt_renderer_get_c_increment(spos[1], spos[2]);
		Plt_Vector2i c2_inc = plt_renderer_get_c_increment(spos[2], spos[0]);
		Plt_Vector2i c3_inc = plt_renderer_get_c_increment(spos[0], spos[1]);
		
		int initial_c1 = plt_renderer_orient2d(spos[1], spos[2], (Plt_Vector2i){bounds_min.x, bounds_min.y});
		int initial_c2 = plt_renderer_orient2d(spos[2], spos[0], (Plt_Vector2i){bounds_min.x, bounds_min.y});
		int initial_c3 = plt_renderer_orient2d(spos[0], spos[1], (Plt_Vector2i){bounds_min.x, bounds_min.y});
		
		/* Half-space triangle rasterization */
		int cy1 = initial_c1; int cy2 = initial_c2; int cy3 = initial_c3;
		for (int y = bounds_min.y; y < bounds_max.y; ++y) {
			int cx1 = cy1; int cx2 = cy2; int cx3 = cy3;
			for (int x = bounds_min.x; x < bounds_max.x; ++x) {
				
				if ((cx1 <= 0) && (cx2 <= 0) && (cx3 <= 0)) {
					float sum = cx1 + cx2 + cx3;
					if (sum == 0) {
						continue;
					}
					
					int framebuffer_pixel_index = y * framebuffer_width + x;
					Plt_Vector3f weights = {cx1 / sum, cx2 / sum, cx3 / sum};
					
					Plt_Vector2i pixel_pos = (Plt_Vector2i){x, y};
					
					float depth = 0;
					depth += (pos[0].z * weights.x) / pos[0].w;
					depth += (pos[1].z * weights.y) / pos[1].w;
					depth += (pos[2].z * weights.z) / pos[2].w;
					
					float depth_sample = depth_buffer[framebuffer_pixel_index];
					if ((depth < 0) || (depth_sample < depth)) {
						continue;
					}
					depth_buffer[framebuffer_pixel_index] = depth;
					
					#if RASTER_LIGHTING_MODEL == 1
					float light_amount = 0.0f;
					light_amount += lighting[0] * weights.x;
					light_amount += lighting[1] * weights.y;
					light_amount += lighting[2] * weights.z;
					#endif
					
					Plt_Color8 tex_color;
					#if RASTER_TEXTURED
						Plt_Vector2f uv = {};
						uv = plt_vector2f_add(uv, plt_vector2f_multiply_scalar(uvs[0], weights.x));
						uv = plt_vector2f_add(uv, plt_vector2f_multiply_scalar(uvs[1], weights.y));
						uv = plt_vector2f_add(uv, plt_vector2f_multiply_scalar(uvs[2], weights.z));
						tex_color = plt_texture_sample(renderer->bound_texture, uv);
					#else
						tex_color = render_color;
					#endif
					
					#if RASTER_LIGHTING_MODEL == 1
						tex_color = plt_color8_multiply_scalar(tex_color, light_amount);
					#endif
					
					framebuffer_pixels[framebuffer_pixel_index] = tex_color;
				}
				
				cx1 += c1_inc.x; cx2 += c2_inc.x; cx3 += c3_inc.x;
			}
			
			cy1 += c1_inc.y; cy2 += c2_inc.y; cy3 += c3_inc.y;
		}
	}
}
