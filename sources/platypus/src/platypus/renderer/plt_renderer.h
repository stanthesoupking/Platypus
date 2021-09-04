#pragma once
#include "platypus/platypus.h"
#include "platypus/framebuffer/plt_framebuffer.h"
#include "platypus/base/allocation/plt_linear_allocator.h"

#define PLT_MAXIMUM_RENDERER_DRAW_CALLS 2048

typedef enum Plt_Renderer_Draw_Call_Type {
	Plt_Renderer_Draw_Call_Type_Draw_Mesh,
	Plt_Renderer_Draw_Call_Type_Draw_Direct_Texture
} Plt_Renderer_Draw_Call_Type;

typedef struct Plt_Renderer_Draw_Call {
	Plt_Renderer_Draw_Call_Type type;
	Plt_Primitive_Type primitive_type;
	
	Plt_Matrix4x4f model;
	Plt_Matrix4x4f view;
	Plt_Matrix4x4f projection;
	
	Plt_Mesh *mesh;
	Plt_Texture *texture;
	Plt_Color8 color;
	
	Plt_Rect rect;
	Plt_Vector2i texture_offset;
	unsigned int depth;
} Plt_Renderer_Draw_Call;

typedef struct Plt_Vertex_Processor Plt_Vertex_Processor;
typedef struct Plt_Triangle_Processor Plt_Triangle_Processor;
typedef struct Plt_Triangle_Rasteriser Plt_Triangle_Rasteriser;
typedef struct Plt_Renderer {
	Plt_Application *application;
	Plt_Framebuffer framebuffer;

	Plt_Linear_Allocator *frame_allocator;

	Plt_Vertex_Processor *vertex_processor;
	Plt_Triangle_Processor *triangle_processor;
	Plt_Triangle_Rasteriser *triangle_rasteriser;
	
	float *depth_buffer;
	unsigned int depth_buffer_width;
	unsigned int depth_buffer_height;
	
	unsigned int draw_call_count;
	Plt_Renderer_Draw_Call draw_calls[PLT_MAXIMUM_RENDERER_DRAW_CALLS];
	
	Plt_Primitive_Type primitive_type;
	unsigned int point_size;
	Plt_Lighting_Model lighting_model;
	Plt_Color8 clear_color;
	Plt_Color8 render_color;	
	Plt_Lighting_Setup lighting_setup;
	
	Plt_Texture *bound_texture;
	
	Plt_Matrix4x4f model_matrix;
	Plt_Matrix4x4f view_matrix;
	Plt_Matrix4x4f projection_matrix;
	Plt_Matrix4x4f mvp_matrix;
} Plt_Renderer;

Plt_Renderer *plt_renderer_create(Plt_Application *application, Plt_Framebuffer framebuffer);
void plt_renderer_destroy(Plt_Renderer **renderer);

void plt_renderer_update_framebuffer(Plt_Renderer *renderer, Plt_Framebuffer framebuffer);
void plt_renderer_rasterise_triangles(Plt_Renderer *renderer);
void plt_renderer_execute(Plt_Renderer *renderer);
