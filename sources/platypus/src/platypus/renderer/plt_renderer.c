#include "plt_renderer.h"

#include <stdlib.h>
#include "platypus/application/plt_application.h"
#include "platypus/base/plt_macros.h"
#include "platypus/base/plt_platform.h"
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
	renderer->frame_allocator = plt_linear_allocator_create(1024 * 1024 * 128); // 64MB

	renderer->vertex_processor = plt_vertex_processor_create();
	renderer->triangle_processor = plt_triangle_processor_create();
	renderer->triangle_rasteriser = plt_triangle_rasteriser_create(renderer, (Plt_Size){ framebuffer.width, framebuffer.height });
	
	renderer->model_matrix =
	renderer->view_matrix =
	renderer->projection_matrix =
	renderer->mvp_matrix = plt_matrix_identity();
	
	renderer->draw_call_count = 0;
	
	renderer->point_size = 1;
	renderer->primitive_type = Plt_Primitive_Type_Triangle;
	renderer->lighting_model = Plt_Lighting_Model_Unlit;
	renderer->render_color = plt_color8_make(255,255,255,255);
	
	renderer->lighting_setup = (Plt_Lighting_Setup) {
		.ambient_lighting = plt_vector3f_make(0.16f, 0.16f, 0.16f),
		.directional_light_count = 1,
		.directional_light_directions = plt_vector3f_make(0.0f, -1.0f, 0.0f),
		.directional_light_amounts = plt_vector3f_make(1.0f, 1.0f, 1.0f)
	};
	
	renderer->bound_texture = NULL;
	
	renderer->depth_buffer_width = 0;
	renderer->depth_buffer_height = 0;
	renderer->depth_buffer = NULL;
	
	plt_renderer_update_framebuffer(renderer, framebuffer);
	
	return renderer;
}

void plt_renderer_destroy(Plt_Renderer **renderer) {
	plt_linear_allocator_destroy(&(*renderer)->frame_allocator);
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

void plt_renderer_rasterise_triangles(Plt_Renderer *renderer) {
	// Rasterise triangles
	plt_timer_start(tr_timer)
	plt_triangle_rasteriser_render_triangles(renderer->triangle_rasteriser);
	plt_linear_allocator_clear(renderer->frame_allocator);
	plt_timer_end(tr_timer, "TRIANGLE_RASTERISER")
}

void plt_renderer_clear(Plt_Renderer *renderer, Plt_Color8 clear_color) {
	Plt_Framebuffer framebuffer = renderer->framebuffer;
	
	// Clear triangle bins
	plt_rasteriser_clear_triangle_bins(renderer->triangle_rasteriser);
}

void plt_renderer_plot_line_low(Plt_Renderer *renderer, Plt_Vector2i p0, Plt_Vector2i p1, Plt_Color8 color) {
	int dx = p1.x - p0.x;
	int dy = p1.y - p0.y;
	int yi = 1;
	if (dy < 0) {
		yi = -1;
		dy = -dy;
	}
	int d = (2 * dy) - dx;
	int y = p0.y;
	
	for (int x = p0.x; x < p1.x; ++x) {
		plt_renderer_poke_pixel(renderer, (Plt_Vector2i){x, y}, color);
		if (d > 0) {
			y += yi;
			d += 2 * (dy - dx);
		} else {
			d += 2 * dy;
		}
	}
}

void plt_renderer_plot_line_high(Plt_Renderer *renderer, Plt_Vector2i p0, Plt_Vector2i p1, Plt_Color8 color) {
	int dx = p1.x - p0.x;
	int dy = p1.y - p0.y;
	int xi = 1;
	if (dx < 0) {
		xi = -1;
		dx = -dx;
	}
	int d = 2 * dx - dy;
	int x = p0.x;
	
	plt_assert(dx >= 0, "dx is not greater than 0");

	for (int y = p0.y; y < p1.y; ++y) {
		plt_renderer_poke_pixel(renderer, (Plt_Vector2i){x, y}, color);
		if (d > 0) {
			x += xi;
			d += 2 * (dx - dy);
		} else {
			d += 2 * dx;
		}
	}
}

void plt_renderer_plot_line(Plt_Renderer *renderer, Plt_Vector2i p0, Plt_Vector2i p1, Plt_Color8 color) {
	if (abs(p1.y - p0.y) < abs(p1.x - p0.x)) {
		if (p0.x > p1.x) {
			plt_renderer_plot_line_low(renderer, p1, p0, color);
		} else {
			plt_renderer_plot_line_low(renderer, p0, p1, color);
		}
	} else {
		if (p0.y > p1.y) {
			plt_renderer_plot_line_high(renderer, p1, p0, color);
		} else {
			plt_renderer_plot_line_high(renderer, p0, p1, color);
		}
	}
}

void plt_renderer_execute_draw_call_draw_mesh(Plt_Renderer *renderer, Plt_Renderer_Draw_Call draw_call) {
	Plt_Vector2i viewport = { renderer->framebuffer.width, renderer->framebuffer.height };
	Plt_Matrix4x4f mvp = plt_matrix_multiply(draw_call.projection, plt_matrix_multiply(draw_call.view, draw_call.model));
	
	switch (draw_call.primitive_type) {
		case Plt_Primitive_Type_Triangle: {
			Plt_Vertex_Processor_Result vp_result = plt_vertex_processor_process_mesh(renderer->vertex_processor, renderer->frame_allocator, renderer->lighting_setup, draw_call.mesh, viewport, draw_call.model, mvp);
			plt_triangle_processor_process_vertex_data(renderer->triangle_processor, renderer->frame_allocator, viewport, draw_call.texture, vp_result, renderer->triangle_rasteriser);
		} break;
			
		case Plt_Primitive_Type_Line: {
			Plt_Vertex_Processor_Result vp_result = plt_vertex_processor_process_mesh(renderer->vertex_processor, renderer->frame_allocator, renderer->lighting_setup, draw_call.mesh, viewport, draw_call.model, mvp);

			// Draw lines
			for (unsigned int i = 0; i < vp_result.vertex_count; i += 3) {
				bool behind_camera = false;
				bool on_screen = false;
				Plt_Vector2i points[3];
				for (unsigned int j = 0; j < 3; ++j) {
					if (vp_result.clipspace_w[i + j] <= 0) {
						behind_camera = true;
						break;
					}
					
					points[j].x = vp_result.screen_positions_x[i + j];
					points[j].y = vp_result.screen_positions_y[i + j];
					
					if ((points[j].x > 0) && (points[j].x < renderer->framebuffer.width) && (points[j].y > 0) && (points[j].y < renderer->framebuffer.height)) {
						on_screen = true;
					}
				}
				
				if (behind_camera || !on_screen) {
					continue;
				}

				plt_renderer_plot_line(renderer, points[0], points[1], draw_call.color);
				plt_renderer_plot_line(renderer, points[1], points[2], draw_call.color);
				plt_renderer_plot_line(renderer, points[2], points[0], draw_call.color);
			}
		} break;
			
		case Plt_Primitive_Type_Point: {
			for (unsigned int i = 0; i < draw_call.mesh->vertex_count; i += 1) {
				Plt_Vector4f pos = {
					draw_call.mesh->position_x[i],
					draw_call.mesh->position_y[i],
					draw_call.mesh->position_z[i],
					1.0f
				};
				
				pos = plt_matrix_multiply_vector4f(mvp, pos);
				
				// Discard behind the camera
				if (pos.z < 0) {
					return;
				}
				
				Plt_Vector2f clip_xy = {pos.x / pos.w, pos.y / pos.w};
				plt_renderer_draw_point(renderer, clip_xy, draw_call.color);
			}
		} break;
	}
}

void plt_renderer_execute_draw_call_draw_direct_texture(Plt_Renderer *renderer, Plt_Renderer_Draw_Call draw_call) {
	Plt_Vector2i bounds_min = {
		plt_clamp(draw_call.rect.x, 0, renderer->framebuffer.width),
		plt_clamp(draw_call.rect.y, 0, renderer->framebuffer.height)
	};

	Plt_Vector2i bounds_max = {
		plt_clamp(draw_call.rect.x + draw_call.rect.width, 0, renderer->framebuffer.width),
		plt_clamp(draw_call.rect.y + draw_call.rect.height, 0, renderer->framebuffer.height)
	};

	Plt_Color8 *pixels = renderer->framebuffer.pixels;
	unsigned int row_length = renderer->framebuffer.width;
		
	Plt_Vector2i p_inc = { 1, 1 };
	Plt_Vector2i tex_pos = draw_call.texture_offset;
	for (unsigned int y = bounds_min.y; y < bounds_max.y; ++y) {
		tex_pos.x = draw_call.texture_offset.x;
		for (unsigned int x = bounds_min.x; x < bounds_max.x; ++x) {
			Plt_Color8 pixel = plt_texture_get_pixel(draw_call.texture, tex_pos);
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

void plt_renderer_execute_draw_call(Plt_Renderer *renderer, Plt_Renderer_Draw_Call draw_call) {
	switch (draw_call.type) {
		case Plt_Renderer_Draw_Call_Type_Draw_Mesh:
			plt_renderer_execute_draw_call_draw_mesh(renderer, draw_call);
			break;
			
		case Plt_Renderer_Draw_Call_Type_Draw_Direct_Texture:
			plt_renderer_execute_draw_call_draw_direct_texture(renderer, draw_call);
			break;
	}
}

void plt_renderer_execute(Plt_Renderer *renderer) {
	// Draw filled meshes first
	for (unsigned int i = 0; i < renderer->draw_call_count; ++i) {
		Plt_Renderer_Draw_Call call = renderer->draw_calls[i];
		if ((call.type == Plt_Renderer_Draw_Call_Type_Draw_Mesh) && (call.primitive_type == Plt_Primitive_Type_Triangle)) {
			plt_renderer_execute_draw_call(renderer, call);
		}
	}
	plt_renderer_rasterise_triangles(renderer);
	plt_linear_allocator_clear(renderer->frame_allocator);
	
	// Draw every other scene element
	for (unsigned int i = 0; i < renderer->draw_call_count; ++i) {
		Plt_Renderer_Draw_Call call = renderer->draw_calls[i];
		if ((call.type == Plt_Renderer_Draw_Call_Type_Draw_Mesh) && (call.primitive_type != Plt_Primitive_Type_Triangle)) {
			plt_renderer_execute_draw_call(renderer, call);
		}
	}
	plt_linear_allocator_clear(renderer->frame_allocator);
	
	// Draw direct calls
	for (unsigned int i = 0; i < renderer->draw_call_count; ++i) {
		Plt_Renderer_Draw_Call call = renderer->draw_calls[i];
		if (call.type == Plt_Renderer_Draw_Call_Type_Draw_Direct_Texture) {
			plt_renderer_execute_draw_call(renderer, renderer->draw_calls[i]);
		}
	}
	plt_linear_allocator_clear(renderer->frame_allocator);
		
	renderer->draw_call_count = 0;
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
	renderer->draw_calls[renderer->draw_call_count++] = (Plt_Renderer_Draw_Call) {
		.type = Plt_Renderer_Draw_Call_Type_Draw_Direct_Texture,
		
		.texture = texture,
		.color = renderer->render_color,
		
		.rect = rect,
		.texture_offset = texture_offset,
		.depth = depth
	};
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
	renderer->draw_calls[renderer->draw_call_count++] = (Plt_Renderer_Draw_Call) {
		.type = Plt_Renderer_Draw_Call_Type_Draw_Mesh,
		.primitive_type = renderer->primitive_type,
		
		.model = renderer->model_matrix,
		.view = renderer->view_matrix,
		.projection = renderer->projection_matrix,
		
		.mesh = mesh,
		.texture = renderer->bound_texture,
		.color = renderer->render_color
	};
}

void plt_renderer_draw_billboard(Plt_Renderer *renderer, Plt_Vector2f size) {
	Plt_Vector4f clipspace = plt_vector4f_make(0, 0, 0, 1);
	clipspace = plt_matrix_multiply_vector4f(renderer->mvp_matrix, clipspace);
	
	if (clipspace.w < 0) {
		// Behind camera, don't draw.
		return;
	}
	
	Plt_Vector3f ndc = plt_vector3f_make(clipspace.x / clipspace.w, clipspace.y / clipspace.w, clipspace.z / clipspace.w);
	
	Plt_Size framebuffer_size = plt_renderer_get_framebuffer_size(renderer);
	Plt_Vector2i screen_space = plt_vector2i_make((ndc.x * 0.5f + 0.5f) * framebuffer_size.width, (ndc.y * 0.5f + 0.5f) * framebuffer_size.height);
	
	// TODO: Get camera FOV
	float perspective_adjustment = sinf(plt_math_deg2rad(50.0f)) / clipspace.z;
	Plt_Vector2f perspective_size = plt_vector2f_make(size.x * perspective_adjustment * 0.5f, size.y * perspective_adjustment * 0.5f);
//	perspective_size = plt_vector2f_make(0.1f, 0.1f);
	
	Plt_Vector2i bounds_min = {
		(perspective_size.x * -(int)framebuffer_size.width) + screen_space.x,
		(perspective_size.x * -(int)framebuffer_size.width) + screen_space.y
	};
	Plt_Vector2i bounds_max = {
		(perspective_size.x * framebuffer_size.width) + screen_space.x,
		(perspective_size.x * framebuffer_size.width) + screen_space.y
	};
	
	Plt_Texture *texture = renderer->bound_texture;
	Plt_Vector2f p_inc = { 0.0f, 0.0f };
	if (texture) {
		p_inc = (Plt_Vector2f){ 1.0f / (float)(perspective_size.x * framebuffer_size.width * 2.0f), 1.0f / (float)(perspective_size.y * framebuffer_size.width * 2.0f) };
	}
	
	Plt_Color8 *pixels = renderer->framebuffer.pixels;
	float *depth_buffer = renderer->depth_buffer;
	Plt_Color8 render_color = plt_color8_make(0, 255, 0, 255);
	Plt_Vector2f tex_pos = { 0.0f, 0.0f };
	
	Plt_Vector2i clamped_bounds_min = {
		plt_clamp(bounds_min.x, 0, framebuffer_size.width),
		plt_clamp(bounds_min.y, 0, framebuffer_size.height)
	};
	Plt_Vector2i clamped_bounds_max = {
		plt_clamp(bounds_max.x, 0, framebuffer_size.width),
		plt_clamp(bounds_max.y, 0, framebuffer_size.height)
	};
	
	float initial_x = plt_max((clamped_bounds_min.x - bounds_min.x) * p_inc.x, 0);
	tex_pos.y = plt_max((clamped_bounds_min.y - bounds_min.y) * p_inc.x, 0);
	
	for (int y = clamped_bounds_min.y; y < clamped_bounds_max.y; ++y) {
		tex_pos.x = initial_x;
		for (int x = clamped_bounds_min.x; x < clamped_bounds_max.x; ++x) {
			Plt_Color8 c = plt_texture_sample(texture, tex_pos);
			if (c.a > 0) {
				float depth_sample = depth_buffer[y * framebuffer_size.width + x];
				if (depth_sample > clipspace.z) {
					depth_buffer[y * framebuffer_size.width + x] = clipspace.z;
					pixels[y * framebuffer_size.width + x] = c;
				}
			}
			tex_pos.x += p_inc.x;
		}
		tex_pos.y += p_inc.y;
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

void plt_renderer_set_lighting_setup(Plt_Renderer* renderer, Plt_Lighting_Setup setup) {
	// Normalise all directional lights
	for (unsigned int i = 0; i < setup.directional_light_count; ++i) {
		setup.directional_light_directions[i] = plt_vector3f_normalize(setup.directional_light_directions[i]);
	}
	renderer->lighting_setup = setup;
}

void plt_renderer_bind_texture(Plt_Renderer *renderer, Plt_Texture *texture) {
	renderer->bound_texture = texture;
}

void plt_renderer_update_mvp(Plt_Renderer *renderer) {
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

Plt_Size plt_renderer_get_framebuffer_size(Plt_Renderer *renderer) {
	return (Plt_Size) { renderer->framebuffer.width, renderer->framebuffer.height };
}
