#pragma once

#include "platypus/renderer/plt_renderer.h"

static inline float plt_renderer_perpendicular_dot_product(Plt_Vector2f a, Plt_Vector2f b) {
	return a.x * b.y - a.y * b.x;
};

static inline int plt_renderer_orient2d(Plt_Vector2i a, Plt_Vector2i b, Plt_Vector2i c) {
	return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

static inline Plt_Vector2i plt_renderer_get_c_increment(Plt_Vector2i a, Plt_Vector2i b) {
	int zero = plt_renderer_orient2d(a, b, (Plt_Vector2i){0, 0});
	
	Plt_Vector2i increment = {
		.x = plt_renderer_orient2d(a, b, (Plt_Vector2i){1, 0}) - zero,
		.y = plt_renderer_orient2d(a, b, (Plt_Vector2i){0, 1}) - zero
	};
	
	return increment;
}
