#pragma once

#include "plt_renderer.h"
#include "platypus/base/neon.h"

static inline float plt_renderer_perpendicular_dot_product(Plt_Vector2f a, Plt_Vector2f b) {
	return a.x * b.y - a.y * b.x;
};

static inline int plt_renderer_orient2d(Plt_Vector2i a, Plt_Vector2i b, Plt_Vector2i c) {
	return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

static inline Plt_Vector2i plt_renderer_get_c_increment(Plt_Vector2i a, Plt_Vector2i b) {
	int zero = plt_renderer_orient2d(a, b, (Plt_Vector2i){0, 0});
	
	Plt_Vector2i increment = {};
	
	increment.x = plt_renderer_orient2d(a, b, (Plt_Vector2i){1, 0}) - zero;
	increment.y = plt_renderer_orient2d(a, b, (Plt_Vector2i){0, 1}) - zero;
	
	return increment;
}

static inline int32x4_t plt_renderer_orient2d_v(int32x4_t a_x, int32x4_t a_y, int32x4_t b_x, int32x4_t b_y, int32x4_t c_x, int32x4_t c_y) {
	return vsubq_s32(vmulq_s32(vsubq_s32(b_x, a_x), vsubq_s32(c_y, a_y)), vmulq_s32(vsubq_s32(b_y, a_y), vsubq_s32(c_x, a_x)));
}

static inline int32x4_t plt_renderer_perp_dot(int32x4_t a_x, int32x4_t a_y, int32x4_t b_x, int32x4_t b_y) {
	return vsubq_s32(vmulq_s32(a_x, b_y), vmulq_s32(a_y, b_x));
}