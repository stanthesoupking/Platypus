#pragma once
#include "platypus/platypus.h"
#include "platypus/framebuffer/plt_framebuffer.h"

typedef struct Plt_Renderer {
	Plt_Application *application;
	Plt_Framebuffer framebuffer;
	
	float *depth_buffer;
	unsigned int depth_buffer_width;
	unsigned int depth_buffer_height;
	
	Plt_Primitive_Type primitive_type;
	unsigned int point_size;
	Plt_Lighting_Model lighting_model;
	Plt_Color8 render_color;

	Plt_Color8 ambient_lighting_color;
	Plt_Color8 directional_lighting_color;
	Plt_Vector3f directional_lighting_direction;
	
	Plt_Texture *bound_texture;
	
	Plt_Matrix4x4f model_matrix;
	Plt_Matrix4x4f view_matrix;
	Plt_Matrix4x4f projection_matrix;
	Plt_Matrix4x4f mvp_matrix;
} Plt_Renderer;

Plt_Renderer *plt_renderer_create(Plt_Application *application, Plt_Framebuffer framebuffer);
void plt_renderer_destroy(Plt_Renderer **renderer);

void plt_renderer_update_framebuffer(Plt_Renderer *renderer, Plt_Framebuffer framebuffer);