#include "plt_renderer.h"

#include <stdlib.h>
#include "platypus/application/plt_application.h"
#include "platypus/base/macros.h"
#include "platypus/base/platform.h"
#include "platypus/mesh/plt_mesh.h"
#include "platypus/renderer/pipeline/plt_vertex_processor.h"
#include "platypus/renderer/pipeline/plt_triangle_processor.h"
#include "platypus/renderer/pipeline/plt_triangle_rasteriser.h"

Plt_Vector2i plt_renderer_clipspace_to_pixel(Plt_Renderer *renderer, Plt_Vector2f p);
void plt_renderer_draw_point(Plt_Renderer *renderer, Plt_Vector2f p, Plt_Color8 color);
void plt_renderer_poke_pixel(Plt_Renderer *renderer, Plt_Vector2i p, Plt_Color8 color);

void plt_renderer_draw_mesh_points(Plt_Renderer *renderer, Plt_Mesh *mesh);
void plt_renderer_draw_mesh_lines(Plt_Renderer *renderer, Plt_Mesh *mesh);
void plt_renderer_draw_mesh_triangles(Plt_Renderer *renderer, Plt_Mesh *mesh);

Plt_Renderer *plt_renderer_create(Plt_Application *application, Plt_Framebuffer framebuffer) {
	Plt_Renderer *renderer = malloc(sizeof(Plt_Renderer));
	
	renderer->application = application;

	renderer->vertex_processor = plt_vertex_processor_create();
	renderer->triangle_processor = plt_triangle_processor_create();
	renderer->triangle_rasteriser = plt_triangle_rasteriser_create(renderer, (Plt_Size){ framebuffer.width, framebuffer.height });
	
	renderer->model_matrix =
	renderer->view_matrix =
	renderer->projection_matrix =
	renderer->mvp_matrix = plt_matrix_identity();
	
	renderer->point_size = 1;
	renderer->primitive_type = Plt_Primitive_Type_Triangle;
	renderer->lighting_model = Plt_Lighting_Model_Unlit;
	renderer->render_color = plt_color8_make(255,255,255,255);

	renderer->ambient_lighting = (Plt_Vector3f){ 0.16f, 0.16f, 0.16f };
	renderer->directional_lighting = (Plt_Vector3f){ 1.0f, 1.0f, 1.0f };
	renderer->directional_lighting_direction = (Plt_Vector3f){ 0.0f, -1.0f, 0.0f };
	
	renderer->bound_texture = NULL;
	
	renderer->depth_buffer_width = 0;
	renderer->depth_buffer_height = 0;
	renderer->depth_buffer = NULL;
	
	plt_renderer_update_framebuffer(renderer, framebuffer);
	
	return renderer;
}

void plt_renderer_destroy(Plt_Renderer **renderer) {
	plt_vertex_processor_destroy(&(*renderer)->vertex_processor);
	plt_triangle_processor_destroy(&(*renderer)->triangle_processor);

	if ((*renderer)->depth_buffer) {
		free((*renderer)->depth_buffer);
	}
	
	free(*renderer);
	*renderer = NULL;
}

void plt_renderer_update_framebuffer(Plt_Renderer *renderer, Plt_Framebuffer framebuffer) {
	renderer->framebuffer = framebuffer;
	plt_triangle_rasteriser_update_framebuffer(renderer->triangle_rasteriser, framebuffer);
	
	if ((renderer->depth_buffer_width != framebuffer.width) || (renderer->depth_buffer_height != framebuffer.height)) {
		if (renderer->depth_buffer) {
			free(renderer->depth_buffer);
		}
		renderer->depth_buffer = malloc(sizeof(float) * framebuffer.width * framebuffer.height);
		renderer->depth_buffer_width = framebuffer.width;
		renderer->depth_buffer_height = framebuffer.height;

		plt_triangle_rasteriser_update_depth_buffer(renderer->triangle_rasteriser, renderer->depth_buffer);
	}
}

void plt_renderer_clear(Plt_Renderer *renderer, Plt_Color8 clear_color) {
	Plt_Framebuffer framebuffer = renderer->framebuffer;
	
#ifdef PLT_PLATFORM_MACOS
	memset_pattern4(framebuffer.pixels, &clear_color, sizeof(Plt_Color8) * framebuffer.width * framebuffer.height);
#else
	int total_pixels = framebuffer.width * framebuffer.height;
	for(int i = 0; i < total_pixels; ++i) {
		framebuffer.pixels[i] = clear_color;
	}
#endif
	
	if (renderer->depth_buffer) {
		float clear_depth = INFINITY;
#ifdef PLT_PLATFORM_MACOS
		memset_pattern4(renderer->depth_buffer, &clear_depth, sizeof(float) * renderer->depth_buffer_width * renderer->depth_buffer_height);
#else
		int total_depth_pixels = framebuffer.width * framebuffer.height;
		for (int i = 0; i < total_depth_pixels; ++i) {
			renderer->depth_buffer[i] = INFINITY;
		}
#endif
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
		plt_renderer_draw_point(renderer, clip_xy, renderer->render_color);
	}
}

void plt_renderer_draw_mesh_lines(Plt_Renderer *renderer, Plt_Mesh *mesh) {
	// TODO: Implement line rendering
}

void plt_renderer_draw_mesh_triangles(Plt_Renderer *renderer, Plt_Mesh *mesh) {
	plt_timer_start(draw_mesh_timer);

	Plt_Vector2i viewport = { renderer->framebuffer.width, renderer->framebuffer.height };
	
	// Process vertices
	plt_timer_start(vp_timer)
	Plt_Vertex_Processor_Result vp_result = plt_vertex_processor_process_mesh(renderer->vertex_processor, mesh, viewport, renderer->model_matrix, renderer->mvp_matrix);
	plt_timer_end(vp_timer, "VERTEX_PROCESSOR")

	// Process triangles
	plt_timer_start(tp_timer)
	Plt_Triangle_Processor_Result tp_result = plt_triangle_processor_process_vertex_data(renderer->triangle_processor, viewport, vp_result);
	plt_timer_end(tp_timer, "TRIANGLE_PROCESSOR")

	// Rasterise triangles
	plt_timer_start(tr_timer)
	plt_triangle_rasteriser_render_triangles(renderer->triangle_rasteriser, tp_result, vp_result);
	plt_timer_end(tr_timer, "TRIANGLE_RASTERISER")
	
	plt_timer_end(draw_mesh_timer, "DRAW_MESH");
}

void plt_renderer_direct_draw_pixel(Plt_Renderer *renderer, Plt_Vector2i position, unsigned int depth, Plt_Color8 color) {
	if ((position.x < 0) || (position.y < 0) || (position.x > renderer->framebuffer.width) || (position.y > renderer->framebuffer.height)) {
		return;
	}
	renderer->framebuffer.pixels[position.y * renderer->framebuffer.width + position.x] = color;
}

void plt_renderer_direct_draw_colored_rect(Plt_Renderer *renderer, Plt_Rect rect, unsigned int depth, Plt_Color8 color) {

}

void plt_renderer_direct_draw_texture(Plt_Renderer *renderer, Plt_Rect rect, unsigned int depth, Plt_Texture *texture) {
	plt_renderer_direct_draw_texture_with_offset(renderer, rect, (Plt_Vector2i){0, 0}, depth, texture);
}

void plt_renderer_direct_draw_texture_with_offset(Plt_Renderer *renderer, Plt_Rect rect, Plt_Vector2i texture_offset, unsigned int depth, Plt_Texture *texture) {
	Plt_Vector2i bounds_min = {
		plt_clamp(rect.x, 0, renderer->framebuffer.width),
		plt_clamp(rect.y, 0, renderer->framebuffer.height)
	};

	Plt_Vector2i bounds_max = {
		plt_clamp(rect.x + rect.width, 0, renderer->framebuffer.width),
		plt_clamp(rect.y + rect.height, 0, renderer->framebuffer.height)
	};

	Plt_Color8 *pixels = renderer->framebuffer.pixels;
	unsigned int row_length = renderer->framebuffer.width;
		
	Plt_Vector2i p_inc = { 1, 1 };
	Plt_Vector2i tex_pos = texture_offset;
	for (unsigned int y = bounds_min.y; y < bounds_max.y; ++y) {
		tex_pos.x = texture_offset.x;
		for (unsigned int x = bounds_min.x; x < bounds_max.x; ++x) {
			Plt_Color8 pixel = plt_texture_get_pixel(texture, tex_pos);
			if (pixel.a == 255) {
				pixels[y * row_length + x] = pixel;
			} else if (pixel.a > 0) {
				// Alpha blend
				pixels[y * row_length + x] = plt_color8_blend(pixels[y * row_length + x], pixel);
			}
			tex_pos.x++;
		}
		tex_pos.y++;
	}
}

void plt_renderer_direct_draw_scaled_texture(Plt_Renderer *renderer, Plt_Rect rect, unsigned int depth, Plt_Texture *texture) {
	Plt_Vector2i bounds_min = {
		plt_clamp(rect.x, 0, renderer->framebuffer.width),
		plt_clamp(rect.y, 0, renderer->framebuffer.height)
	};

	Plt_Vector2i bounds_max = {
		plt_clamp(rect.x + rect.width, 0, renderer->framebuffer.width),
		plt_clamp(rect.y + rect.height, 0, renderer->framebuffer.height)
	};

	Plt_Color8 render_color = renderer->render_color;
	Plt_Color8 *pixels = renderer->framebuffer.pixels;
	unsigned int row_length = renderer->framebuffer.width;
	
	Plt_Texture *bound_texture = renderer->bound_texture;
	
	Plt_Vector2f p_inc = { 0.0f, 0.0f };
	if (bound_texture) {
		p_inc = (Plt_Vector2f){ 1.0f / (float)rect.width, 1.0f / (float)rect.height };
	}
	
	Plt_Vector2f tex_pos = { 0.0f, 0.0f };
	for (unsigned int y = bounds_min.y; y < bounds_max.y; ++y) {
		tex_pos.x = 0.0f;
		for (unsigned int x = bounds_min.x; x < bounds_max.x; ++x) {
			
			Plt_Color8 color;
			if (bound_texture) {
				color = plt_texture_sample(bound_texture, tex_pos);
			} else {
				color = render_color;
			}
			
			pixels[y * row_length + x] = color;
			
			tex_pos.x += p_inc.x;
		}
		tex_pos.y += p_inc.y;
	}
}

void plt_renderer_direct_draw_text(Plt_Renderer *renderer, Plt_Vector2i position, Plt_Font *font, const char *text) {
	Plt_Size character_size = plt_font_get_character_size(font);
	Plt_Texture *font_texture = plt_font_get_texture(font);
	
	Plt_Rect view_rect = {
		.x = position.x,
		.y = position.y,
		.width = character_size.width,
		.height = character_size.height
	};
	
	char c = *text++;
	while (c) {
		Plt_Rect char_rect = plt_font_get_rect_for_character(font, c);
		Plt_Vector2i texture_offset = { char_rect.x, char_rect.y };
		plt_renderer_direct_draw_texture_with_offset(renderer, view_rect, texture_offset, 0, font_texture);
		view_rect.x += character_size.width;
		c = *text++;
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

void plt_renderer_set_render_color(Plt_Renderer *renderer, Plt_Color8 color) {
	renderer->render_color = color;
}

void plt_renderer_bind_texture(Plt_Renderer *renderer, Plt_Texture *texture) {
	renderer->bound_texture = texture;
}

void plt_renderer_set_ambient_lighting(Plt_Renderer *renderer, Plt_Vector3f value) {
	renderer->ambient_lighting = value;
}

void plt_renderer_set_directional_lighting(Plt_Renderer *renderer, Plt_Vector3f value) {
	renderer->directional_lighting = value;
}

void plt_renderer_set_directional_lighting_direction(Plt_Renderer *renderer, Plt_Vector3f direction) {
	renderer->directional_lighting_direction = direction;
}

void plt_renderer_update_mvp(Plt_Renderer *renderer) {
	// renderer->mvp_matrix = plt_matrix_multiply(plt_matrix_multiply(renderer->projection_matrix, renderer->view_matrix), renderer->model_matrix);
	Plt_Matrix4x4f mv = plt_matrix_multiply(renderer->view_matrix, renderer->model_matrix);

	renderer->mvp_matrix = plt_matrix_multiply(renderer->projection_matrix, mv);
}

void plt_renderer_set_model_matrix(Plt_Renderer *renderer, Plt_Matrix4x4f matrix) {
	renderer->model_matrix = matrix;
	plt_renderer_update_mvp(renderer);
}

void plt_renderer_set_view_matrix(Plt_Renderer *renderer, Plt_Matrix4x4f matrix) {
	renderer->view_matrix = matrix;
	plt_renderer_update_mvp(renderer);
}

void plt_renderer_set_projection_matrix(Plt_Renderer *renderer, Plt_Matrix4x4f matrix) {
	renderer->projection_matrix = matrix;
	plt_renderer_update_mvp(renderer);
}

Plt_Vector2i plt_renderer_get_framebuffer_size(Plt_Renderer *renderer) {
	return (Plt_Vector2i) { renderer->framebuffer.width, renderer->framebuffer.height };
}
