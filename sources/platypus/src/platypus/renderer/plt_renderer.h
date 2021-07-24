#pragma once
#include "platypus/platypus.h"
#include "platypus/framebuffer/plt_framebuffer.h"

Plt_Renderer *plt_renderer_create(Plt_Application *application, Plt_Framebuffer *framebuffer);
void plt_renderer_destroy(Plt_Renderer **renderer);