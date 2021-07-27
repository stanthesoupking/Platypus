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

float plt_renderer_orient2d(Plt_Vector2f a, Plt_Vector2f b, Plt_Vector2f c)
{
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

void plt_renderer_draw_mesh_triangles(Plt_Renderer *renderer, Plt_Mesh *mesh) {
	for (unsigned int i = 0; i < mesh->vertex_count; i += 3) {
		Plt_Vector4f pos[3];
		Plt_Vector4f wpos[3];
		Plt_Vector2i spos[3];
		Plt_Vector2f uvs[3];

		for (unsigned int j = 0; j < 3; ++j) {
			pos[j] = (Plt_Vector4f) {
				mesh->position_x[i + j],
				mesh->position_y[i + j],
				mesh->position_z[i + j],
				1.0f
			};
			
			uvs[j] = plt_mesh_get_uv(mesh, i + j);

			pos[j] = plt_matrix_multiply_vector4f(renderer->mvp_matrix, pos[j]);
			wpos[j] = (Plt_Vector4f){ pos[j].x / pos[j].w, pos[j].y / pos[j].w, pos[j].z / pos[j].w, 1.0f };
			spos[j] = plt_renderer_clipspace_to_pixel(renderer, (Plt_Vector2f){wpos[j].x, wpos[j].y});
		}

		Plt_Vector2i bounds_min = spos[0];
		Plt_Vector2i bounds_max = spos[0];
		for (unsigned int j = 1; j < 3; ++j) {
			bounds_min.x = plt_min(spos[j].x, bounds_min.x);
			bounds_min.y = plt_min(spos[j].y, bounds_min.y);
			bounds_max.x = plt_max(spos[j].x, bounds_max.x);
			bounds_max.y = plt_max(spos[j].y, bounds_max.y);
		}

		bounds_min.x = plt_max(bounds_min.x, 0);
		bounds_min.y = plt_max(bounds_min.y, 0);
		bounds_max.x = plt_min(bounds_max.x, renderer->framebuffer->width);
		bounds_max.y = plt_min(bounds_max.y, renderer->framebuffer->height);

		// Half-space triangle rasterization
		for (int y = bounds_min.y; y < bounds_max.y; ++y) {
			for (int x = bounds_min.x; x < bounds_max.x; ++x) {
				Plt_Vector2f p = { x, y };
				float c1 = plt_renderer_orient2d((Plt_Vector2f){spos[1].x,spos[1].y}, (Plt_Vector2f){spos[2].x,spos[2].y}, p);
				float c2 = plt_renderer_orient2d((Plt_Vector2f){spos[2].x,spos[2].y}, (Plt_Vector2f){spos[0].x,spos[0].y}, p);
				float c3 = plt_renderer_orient2d((Plt_Vector2f){spos[0].x,spos[0].y}, (Plt_Vector2f){spos[1].x,spos[1].y}, p);

				if (((c1 <= 0) && (c2 <= 0) && (c3 <= 0)) || ((c1 >= 0) && (c2 >= 0) && (c3 >= 0))) {
					float sum = c1 + c2 + c3;
					Plt_Vector3f weights = {c1 / sum, c2 / sum, c3 / sum};

					Plt_Vector4f world_pos = {};
					world_pos = plt_vector4f_add(world_pos, plt_vector4f_multiply_scalar(pos[0], weights.x));
					world_pos = plt_vector4f_add(world_pos, plt_vector4f_multiply_scalar(pos[1], weights.y));
					world_pos = plt_vector4f_add(world_pos, plt_vector4f_multiply_scalar(pos[2], weights.z));

					world_pos.x /= world_pos.w;
					world_pos.y /= world_pos.w;
					
					Plt_Vector2i pixel_pos = (Plt_Vector2i){x, y};
					
					float depth = world_pos.z;

					if (depth < 0) {
						continue;
					}

					float depth_sample = plt_texture_get_pixel(renderer->depth_texture, pixel_pos).x;
					if (depth_sample < depth) {
						continue;
					}
					
					Plt_Vector2f uv = {};
					uv = plt_vector2f_add(uv, plt_vector2f_multiply_scalar(uvs[0], weights.x));
					uv = plt_vector2f_add(uv, plt_vector2f_multiply_scalar(uvs[1], weights.y));
					uv = plt_vector2f_add(uv, plt_vector2f_multiply_scalar(uvs[2], weights.z));

					Plt_Vector4f tex_color = renderer->bound_texture ? plt_texture_sample(renderer->bound_texture, uv) : (Plt_Vector4f){1,0,1,1};
					
					plt_texture_set_pixel(renderer->depth_texture, pixel_pos, (Plt_Vector4f){depth, 0, 0, 0});

					plt_renderer_poke_pixel(renderer, pixel_pos, plt_color8_make(tex_color.z * 255, tex_color.y * 255, tex_color.x * 255, 255));
				}
			}
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
