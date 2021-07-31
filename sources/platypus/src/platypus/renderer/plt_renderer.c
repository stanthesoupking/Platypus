#include "plt_renderer.h"

#include <stdlib.h>
#include "platypus/application/plt_application.h"
#include "platypus/base/macros.h"
#include "platypus/base/neon.h"

Plt_Vector2i plt_renderer_clipspace_to_pixel(Plt_Renderer *renderer, Plt_Vector2f p);
void plt_renderer_draw_point(Plt_Renderer *renderer, Plt_Vector2f p, Plt_Color8 color);
void plt_renderer_poke_pixel(Plt_Renderer *renderer, Plt_Vector2i p, Plt_Color8 color);

void plt_renderer_draw_mesh_points(Plt_Renderer *renderer, Plt_Mesh *mesh);
void plt_renderer_draw_mesh_lines(Plt_Renderer *renderer, Plt_Mesh *mesh);
void plt_renderer_draw_mesh_triangles(Plt_Renderer *renderer, Plt_Mesh *mesh);

Plt_Renderer *plt_renderer_create(Plt_Application *application, Plt_Framebuffer framebuffer) {
	Plt_Renderer *renderer = malloc(sizeof(Plt_Renderer));
	
	renderer->application = application;
	
	renderer->model_matrix =
	renderer->view_matrix =
	renderer->projection_matrix =
	renderer->mvp_matrix = plt_matrix_identity();
	
	renderer->point_size = 1;
	renderer->primitive_type = Plt_Primitive_Type_Triangle;
	renderer->lighting_model = Plt_Lighting_Model_Unlit;
	
	renderer->bound_texture = NULL;
	
	renderer->depth_buffer_width = 0;
	renderer->depth_buffer_height = 0;
	renderer->depth_buffer = NULL;
	
	plt_renderer_update_framebuffer(renderer, framebuffer);
	
	return renderer;
}

void plt_renderer_destroy(Plt_Renderer **renderer) {
	if ((*renderer)->depth_buffer) {
		free((*renderer)->depth_buffer);
	}
	
	free(*renderer);
	*renderer = NULL;
}

void plt_renderer_update_framebuffer(Plt_Renderer *renderer, Plt_Framebuffer framebuffer) {
	renderer->framebuffer = framebuffer;
	
	if ((renderer->depth_buffer_width != framebuffer.width) || (renderer->depth_buffer_height != framebuffer.height)) {
		if (renderer->depth_buffer) {
			free(renderer->depth_buffer);
		}
		renderer->depth_buffer = malloc(sizeof(float) * framebuffer.width * framebuffer.height);
		renderer->depth_buffer_width = framebuffer.width;
		renderer->depth_buffer_height = framebuffer.height;
	}
}

void plt_renderer_clear(Plt_Renderer *renderer, Plt_Color8 clear_color) {
	Plt_Framebuffer framebuffer = renderer->framebuffer;
	
#ifdef __APPLE__
	memset_pattern4(framebuffer.pixels, &clear_color, sizeof(Plt_Color8) * framebuffer.width * framebuffer.height);
#else
	int total_pixels = framebuffer.width * framebuffer.height;
	for(int i = 0; i < total_pixels; ++i) {
		framebuffer.pixels[i] = clear_color;
	}
#endif
	
	if (renderer->depth_buffer) {
		float clear_depth = INFINITY;
		memset_pattern4(renderer->depth_buffer, &clear_depth, sizeof(float) * renderer->depth_buffer_width * renderer->depth_buffer_height);
	}
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

void plt_renderer_draw_mesh_triangles(Plt_Renderer *renderer, Plt_Mesh *mesh) {
	if (renderer->bound_texture) {
		if (renderer->lighting_model == Plt_Lighting_Model_Unlit) {
			_draw_triangle_textured_unlit(renderer, mesh);
		} else {
			_draw_triangle_textured_lit(renderer, mesh);
		}
	} else {
		if (renderer->lighting_model == Plt_Lighting_Model_Unlit) {
			_draw_triangle_untextured_unlit(renderer, mesh);
		} else {
			_draw_triangle_untextured_lit(renderer, mesh);
		}
	}
}

void plt_renderer_draw_mesh(Plt_Renderer *renderer, Plt_Mesh *mesh) {
	Plt_Framebuffer framebuffer = renderer->framebuffer;
	
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
	Plt_Framebuffer framebuffer = renderer->framebuffer;
	
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
	if ((p.x < 0) || (p.y < 0) || (p.x >= renderer->framebuffer.width) || (p.y >= renderer->framebuffer.height)) {
		return;
	}
	
	renderer->framebuffer.pixels[p.y * renderer->framebuffer.width + p.x] = color;
}

Plt_Vector2i plt_renderer_clipspace_to_pixel(Plt_Renderer *renderer, Plt_Vector2f p) {
	return (Plt_Vector2i) {
		(p.x + 1.0f) * 0.5f * renderer->framebuffer.width,
		(p.y + 1.0f) * 0.5f * renderer->framebuffer.height,
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

void plt_renderer_set_lighting_model(Plt_Renderer *renderer, Plt_Lighting_Model model) {
	renderer->lighting_model = model;
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
	return (Plt_Vector2i) { renderer->framebuffer.width, renderer->framebuffer.height };
}
