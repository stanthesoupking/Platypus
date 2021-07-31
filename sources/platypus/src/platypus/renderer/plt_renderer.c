#include "plt_renderer.h"

#include <stdlib.h>
#include "platypus/application/plt_application.h"
#include "platypus/base/macros.h"
#include "platypus/base/neon.h"

typedef struct Plt_Renderer {
	Plt_Application *application;
	Plt_Framebuffer *framebuffer;
	Plt_Texture *depth_texture;
	
	Plt_Primitive_Type primitive_type;
	unsigned int point_size;
	
	Plt_Texture *bound_texture;
	
	Plt_Matrix4x4f model_matrix;
	Plt_Matrix4x4f view_matrix;
	Plt_Matrix4x4f projection_matrix;
	Plt_Matrix4x4f mvp_matrix;
} Plt_Renderer;

Plt_Vector2i plt_renderer_clipspace_to_pixel(Plt_Renderer *renderer, Plt_Vector2f p);
void plt_renderer_draw_point(Plt_Renderer *renderer, Plt_Vector2f p, Plt_Color8 color);
void plt_renderer_poke_pixel(Plt_Renderer *renderer, Plt_Vector2i p, Plt_Color8 color);

void plt_renderer_draw_mesh_points(Plt_Renderer *renderer, Plt_Mesh *mesh);
void plt_renderer_draw_mesh_lines(Plt_Renderer *renderer, Plt_Mesh *mesh);
void plt_renderer_draw_mesh_triangles(Plt_Renderer *renderer, Plt_Mesh *mesh);

Plt_Renderer *plt_renderer_create(Plt_Application *application, Plt_Framebuffer *framebuffer) {
	Plt_Renderer *renderer = malloc(sizeof(Plt_Renderer));
	
	renderer->application = application;
	renderer->framebuffer = framebuffer;
	
	renderer->model_matrix =
	renderer->view_matrix =
	renderer->projection_matrix =
	renderer->mvp_matrix = plt_matrix_identity();
	
	renderer->point_size = 1;
	renderer->primitive_type = Plt_Primitive_Type_Triangle;
	
	renderer->bound_texture = NULL;
	
	renderer->depth_texture = NULL;
	
	return renderer;
}

void plt_renderer_destroy(Plt_Renderer **renderer) {
	if ((*renderer)->depth_texture) {
		plt_texture_destroy(&(*renderer)->depth_texture);
	}
	
	free(*renderer);
	*renderer = NULL;
}

void plt_renderer_clear(Plt_Renderer *renderer, Plt_Color8 clear_color) {
	Plt_Framebuffer framebuffer = *renderer->framebuffer;
	
#ifdef __APPLE__
	memset_pattern4(framebuffer.pixels, &clear_color, sizeof(Plt_Color8) * framebuffer.width * framebuffer.height);
#else
	int total_pixels = framebuffer.width * framebuffer.height;
	for(int i = 0; i < total_pixels; ++i) {
		framebuffer.pixels[i] = clear_color;
	}
#endif
	
	float clear_depth = INFINITY;
	plt_texture_clear(renderer->depth_texture, (Plt_Vector4f){clear_depth});
}

void plt_renderer_draw_mesh_points(Plt_Renderer *renderer, Plt_Mesh *mesh) {
	for (unsigned int i = 0; i < mesh->vertex_count; i += 1) {
		Plt_Vector4f pos = {
			mesh->position_x[i],
			mesh->position_y[i],
			mesh->position_z[i],
			1.0f
		};
		
		pos = plt_matrix_multiply_vector4f(renderer->mvp_matrix, pos);
		
		// Discard behind the camera
		if (pos.z < 0) {
			return;
		}
		
		Plt_Vector2f clip_xy = {pos.x / pos.w, pos.y / pos.w};
		plt_renderer_draw_point(renderer, clip_xy, plt_color8_make(255, 0, 0, 255));
	}
}

void plt_renderer_draw_mesh_lines(Plt_Renderer *renderer, Plt_Mesh *mesh) {
	// TODO: Implement line rendering
}

float plt_renderer_perpendicular_dot_product(Plt_Vector2f a, Plt_Vector2f b) {
	return a.x * b.y - a.y * b.x;
};

int plt_renderer_orient2d(Plt_Vector2i a, Plt_Vector2i b, Plt_Vector2i c)
{
	return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
	//	return vsubq_s32(vmulq_s32(vsubq_s32(b_x, a_x), vsubq_s32(c_y, a_y)), vmulq_s32(vsubq_s32(b_y, a_y), vsubq_s32(c_x, a_x)));
}

Plt_Vector2i plt_renderer_get_c_increment(Plt_Vector2i a, Plt_Vector2i b) {
	int zero = plt_renderer_orient2d(a, b, (Plt_Vector2i){0, 0});
	
	Plt_Vector2i increment = {};
	
	increment.x = plt_renderer_orient2d(a, b, (Plt_Vector2i){1, 0}) - zero;
	increment.y = plt_renderer_orient2d(a, b, (Plt_Vector2i){0, 1}) - zero;
	
	return increment;
}

int32x4_t plt_renderer_orient2d_v(int32x4_t a_x, int32x4_t a_y, int32x4_t b_x, int32x4_t b_y, int32x4_t c_x, int32x4_t c_y)
{
	return vsubq_s32(vmulq_s32(vsubq_s32(b_x, a_x), vsubq_s32(c_y, a_y)), vmulq_s32(vsubq_s32(b_y, a_y), vsubq_s32(c_x, a_x)));
}

int32x4_t plt_renderer_perp_dot(int32x4_t a_x, int32x4_t a_y, int32x4_t b_x, int32x4_t b_y)
{
	return vsubq_s32(vmulq_s32(a_x, b_y), vmulq_s32(a_y, b_x));
}

void plt_renderer_draw_mesh_triangles(Plt_Renderer *renderer, Plt_Mesh *mesh) {
	Plt_Vector3f light_direction = plt_vector3f_normalize((Plt_Vector3f){-0.75f,-0.25f,0});
	float ambient_light = 0.2f;
	
	for (unsigned int i = 0; i < mesh->vertex_count; i += 3) {
		Plt_Vector4f pos[3];
		
		Plt_Vector2i spos[3];
		int32x4_t spos_x[3];
		int32x4_t spos_y[3];
		
		Plt_Vector4f wpos[3];
		Plt_Vector2f uvs[3];
		Plt_Vector3f normals[3];
		Plt_Vector3f face_normal = {};
		float lighting[3];
		
		bool visible = false;
		for (unsigned int j = 0; j < 3; ++j) {
			pos[j] = (Plt_Vector4f) {
				mesh->position_x[i + j],
				mesh->position_y[i + j],
				mesh->position_z[i + j],
				1.0f
			};
			
			uvs[j] = plt_mesh_get_uv(mesh, i + j);
			
			Plt_Vector3f model_normal = plt_mesh_get_normal(mesh, i + j);
			Plt_Vector4f world_normal = plt_matrix_multiply_vector4f(renderer->model_matrix, (Plt_Vector4f){model_normal.x, model_normal.y, model_normal.z, 0.0f});
			normals[j] = plt_vector3f_normalize((Plt_Vector3f){world_normal.x, world_normal.y, world_normal.z});
			
			face_normal = plt_vector3f_add(face_normal, normals[j]);
			
			float light_amount = plt_max(plt_vector3f_dot_product(normals[j], light_direction), 0);
			lighting[j] = plt_clamp(light_amount + ambient_light, 0.0f, 1.0f);
			
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
		
		bounds_min.x = plt_clamp(bounds_min.x, 0, (int)renderer->framebuffer->width);
		bounds_min.y = plt_clamp(bounds_min.y, 0, (int)renderer->framebuffer->height);
		bounds_max.x = plt_clamp(bounds_max.x, 0, (int)renderer->framebuffer->width);
		bounds_max.y = plt_clamp(bounds_max.y, 0, (int)renderer->framebuffer->height);
		
		Plt_Vector2i c1_inc = plt_renderer_get_c_increment(spos[1], spos[2]);
		Plt_Vector2i c2_inc = plt_renderer_get_c_increment(spos[2], spos[0]);
		Plt_Vector2i c3_inc = plt_renderer_get_c_increment(spos[0], spos[1]);
		
		int initial_c1 = plt_renderer_orient2d(spos[1], spos[2], (Plt_Vector2i){bounds_min.x, bounds_min.y});
		int initial_c2 = plt_renderer_orient2d(spos[2], spos[0], (Plt_Vector2i){bounds_min.x, bounds_min.y});
		int initial_c3 = plt_renderer_orient2d(spos[0], spos[1], (Plt_Vector2i){bounds_min.x, bounds_min.y});
		
		// Half-space triangle rasterization
		int cy1 = initial_c1; int cy2 = initial_c2; int cy3 = initial_c3;
		for (int y = bounds_min.y; y < bounds_max.y; ++y) {
			int cx1 = cy1; int cx2 = cy2; int cx3 = cy3;
			for (int x = bounds_min.x; x < bounds_max.x; ++x) {
				
				if ((cx1 <= 0) && (cx2 <= 0) && (cx3 <= 0)) {
					float sum = cx1 + cx2 + cx3;
					if (sum == 0) {
						continue;
					}
					Plt_Vector3f weights = {cx1 / sum, cx2 / sum, cx3 / sum};
					//
					//						Plt_Vector4f world_pos = {};
					//						world_pos = plt_vector4f_add(world_pos, plt_vector4f_multiply_scalar(pos[0], weights.x));
					//						world_pos = plt_vector4f_add(world_pos, plt_vector4f_multiply_scalar(pos[1], weights.y));
					//						world_pos = plt_vector4f_add(world_pos, plt_vector4f_multiply_scalar(pos[2], weights.z));
					//
					//						world_pos.x /= world_pos.w;
					//						world_pos.y /= world_pos.w;
					//
					//
					//						float depth = world_pos.z;
					//						float depth_sample = plt_texture_get_pixel(renderer->depth_texture, pixel_pos).x;
					//						if ((depth < 0) || (depth_sample < depth)) {
					//							continue;
					//						}
					//						plt_texture_set_pixel(renderer->depth_texture, pixel_pos, (Plt_Vector4f){depth, 0, 0, 0});
					//
					//						Plt_Vector3f normal = {};
					//						normal = plt_vector3f_add(normal, plt_vector3f_multiply_scalar(normals[0], weights.x));
					//						normal = plt_vector3f_add(normal, plt_vector3f_multiply_scalar(normals[1], weights.y));
					//						normal = plt_vector3f_add(normal, plt_vector3f_multiply_scalar(normals[2], weights.z));
					//
					//						float light_amount = 0.0f;
					//						light_amount += lighting[0] * weights.x;
					//						light_amount += lighting[1] * weights.y;
					//						light_amount += lighting[2] * weights.z;
					//
					//						Plt_Vector2f uv = {};
					//						uv = plt_vector2f_add(uv, plt_vector2f_multiply_scalar(uvs[0], weights.x));
					//						uv = plt_vector2f_add(uv, plt_vector2f_multiply_scalar(uvs[1], weights.y));
					//						uv = plt_vector2f_add(uv, plt_vector2f_multiply_scalar(uvs[2], weights.z));
					//
					//						Plt_Vector4f tex_color = renderer->bound_texture ? plt_texture_sample(renderer->bound_texture, uv) : (Plt_Vector4f){1,0,1,1};
					//
					//						Plt_Vector4f lit_color = plt_vector4f_multiply_scalar(tex_color, 1.0f);
					//
					Plt_Vector2i pixel_pos = (Plt_Vector2i){x, y};
					plt_renderer_poke_pixel(renderer, pixel_pos, (Plt_Color8){255, 255, 255, 255});
					plt_renderer_poke_pixel(renderer, pixel_pos, (Plt_Color8){weights.x * 255, weights.y * 255, weights.z * 255, 255});
				}
				
				cx1 += c1_inc.x; cx2 += c2_inc.x; cx3 += c3_inc.x;
			}
			
			cy1 += c1_inc.y; cy2 += c2_inc.y; cy3 += c3_inc.y;
		}
	}
}

void plt_renderer_draw_mesh(Plt_Renderer *renderer, Plt_Mesh *mesh) {
	Plt_Framebuffer framebuffer = *renderer->framebuffer;
	
	switch (renderer->primitive_type) {
		case Plt_Primitive_Type_Point:
			plt_renderer_draw_mesh_points(renderer, mesh);
			break;
			
		case Plt_Primitive_Type_Line:
			plt_renderer_draw_mesh_lines(renderer, mesh);
			break;
			
		case Plt_Primitive_Type_Triangle:
			plt_renderer_draw_mesh_triangles(renderer, mesh);
			break;
	}
}

void plt_renderer_draw_point(Plt_Renderer *renderer, Plt_Vector2f p, Plt_Color8 color) {
	Plt_Framebuffer framebuffer = *renderer->framebuffer;
	
	Plt_Vector2i origin_pixel = plt_renderer_clipspace_to_pixel(renderer, p);
	
	if (renderer->point_size == 1) {
		plt_renderer_poke_pixel(renderer, (Plt_Vector2i){origin_pixel.x,origin_pixel.y}, color);
	} else {
		unsigned int half_size = renderer->point_size / 2;
		Plt_Vector2i start = {origin_pixel.x - half_size, origin_pixel.y - half_size};
		Plt_Vector2i end = {origin_pixel.x + half_size, origin_pixel.y + half_size};
		
		for(int x = start.x; x < end.x; ++x) {
			for(int y = start.y; y < end.y; ++y) {
				plt_renderer_poke_pixel(renderer, (Plt_Vector2i){x,y}, color);
			}
		}
	}
}

void plt_renderer_poke_pixel(Plt_Renderer *renderer, Plt_Vector2i p, Plt_Color8 color) {
	if ((p.x < 0) || (p.y < 0) || (p.x >= renderer->framebuffer->width) || (p.y >= renderer->framebuffer->height)) {
		return;
	}
	
	renderer->framebuffer->pixels[p.y * renderer->framebuffer->width + p.x] = color;
}

Plt_Vector2i plt_renderer_clipspace_to_pixel(Plt_Renderer *renderer, Plt_Vector2f p) {
	return (Plt_Vector2i) {
		(p.x + 1.0f) * 0.5f * renderer->framebuffer->width,
		(p.y + 1.0f) * 0.5f * renderer->framebuffer->height,
	};
}

void plt_renderer_present(Plt_Renderer *renderer) {
	plt_application_present(renderer->application);
}

void plt_renderer_set_primitive_type(Plt_Renderer *renderer, Plt_Primitive_Type primitive_type) {
	renderer->primitive_type = primitive_type;
}

void plt_renderer_set_point_size(Plt_Renderer *renderer, unsigned int size) {
	renderer->point_size = size;
}

void plt_renderer_bind_texture(Plt_Renderer *renderer, Plt_Texture *texture) {
	renderer->bound_texture = texture;
}

void plt_renderer_set_model_matrix(Plt_Renderer *renderer, Plt_Matrix4x4f matrix) {
	renderer->model_matrix = matrix;
	renderer->mvp_matrix = plt_matrix_multiply(renderer->projection_matrix, plt_matrix_multiply(renderer->view_matrix, renderer->model_matrix));
}

void plt_renderer_set_view_matrix(Plt_Renderer *renderer, Plt_Matrix4x4f matrix) {
	renderer->view_matrix = matrix;
	renderer->mvp_matrix = plt_matrix_multiply(renderer->projection_matrix, plt_matrix_multiply(renderer->view_matrix, renderer->model_matrix));
}

void plt_renderer_set_projection_matrix(Plt_Renderer *renderer, Plt_Matrix4x4f matrix) {
	renderer->projection_matrix = matrix;
	renderer->mvp_matrix = plt_matrix_multiply(renderer->projection_matrix, plt_matrix_multiply(renderer->view_matrix, renderer->model_matrix));
}

Plt_Vector2i plt_renderer_get_framebuffer_size(Plt_Renderer *renderer) {
	return (Plt_Vector2i) { renderer->framebuffer->width, renderer->framebuffer->height };
}

void plt_renderer_set_depth_texture(Plt_Renderer *renderer, Plt_Texture *texture) {
	if (renderer->depth_texture) {
		plt_texture_destroy(&renderer->depth_texture);
	}
	renderer->depth_texture = texture;
}

Plt_Texture *plt_renderer_get_depth_texture(Plt_Renderer *renderer) {
	return renderer->depth_texture;
}
