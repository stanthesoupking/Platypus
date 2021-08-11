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
