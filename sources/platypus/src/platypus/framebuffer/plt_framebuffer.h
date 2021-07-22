#pragma once
#include "platypus/platypus.h"

typedef struct Plt_Framebuffer {
	Plt_Color8 *pixels;
	unsigned int width, height;
} Plt_Framebuffer;

void plt_framebuffer_clear(Plt_Framebuffer framebuffer, Plt_Color8 color);