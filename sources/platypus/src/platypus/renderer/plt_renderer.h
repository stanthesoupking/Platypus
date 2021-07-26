#pragma once
#include "platypus/platypus.h"
#include "platypus/framebuffer/plt_framebuffer.h"

Plt_Renderer *plt_renderer_create(Plt_Application *application, Plt_Framebuffer *framebuffer);
void plt_renderer_destroy(Plt_Renderer **renderer);

void plt_renderer_set_depth_texture(Plt_Renderer *renderer, Plt_Texture *texture);
Plt_Texture *plt_renderer_get_depth_texture(Plt_Renderer *renderer);