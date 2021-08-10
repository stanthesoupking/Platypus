#pragma once

#include "platypus/framebuffer/plt_framebuffer.h"
#include "plt_triangle_processor.h"
#include "plt_vertex_processor.h"

typedef struct Plt_Triangle_Rasteriser Plt_Triangle_Rasteriser;

Plt_Triangle_Rasteriser *plt_triangle_rasteriser_create(Plt_Renderer *renderer, Plt_Size viewport_size);
void plt_triangle_rasteriser_destroy(Plt_Triangle_Rasteriser **rasteriser);

void plt_triangle_rasteriser_update_framebuffer(Plt_Triangle_Rasteriser *rasteriser, Plt_Framebuffer framebuffer);
void plt_triangle_rasteriser_update_depth_buffer(Plt_Triangle_Rasteriser *rasteriser, float *depth_buffer);

void plt_triangle_rasteriser_render_triangles(Plt_Triangle_Rasteriser *rasteriser, Plt_Triangle_Processor_Result tp_result, Plt_Vertex_Processor_Result vp_result);