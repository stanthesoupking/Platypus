#include "platypus/platypus.h"

Plt_Color8 plt_color8_make(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
	return (Plt_Color8){b, g, r, a};
}

Plt_Color8 plt_color8_add(Plt_Color8 a, Plt_Color8 b) {
	return (Plt_Color8){
		plt_min((int)a.b + (int)b.b, 255),
		plt_min((int)a.g + (int)b.g, 255),
		plt_min((int)a.r + (int)b.r, 255),
		plt_min((int)a.a + (int)b.a, 255)
	};
}

Plt_Color8 plt_color8_multiply_vector3f(Plt_Color8 a, Plt_Vector3f b) {
	return (Plt_Color8){
		plt_clamp(((float)a.b * b.z), 0, 255),
		plt_clamp(((float)a.g * b.y), 0, 255),
		plt_clamp(((float)a.r * b.x), 0, 255),
		a.a
	};
}

Plt_Color8 plt_color8_multiply(Plt_Color8 a, Plt_Color8 b) {
	return (Plt_Color8){
		(a.b * b.b) / 255,
		(a.g * b.g) / 255,
		(a.r * b.r) / 255,
		(a.a * b.a) / 255
	};
}

Plt_Color8 plt_color8_multiply_scalar(Plt_Color8 color, float s) {
	return (Plt_Color8){
		plt_min((float)color.b * (float)s, 255),
		plt_min((float)color.g * (float)s, 255),
		plt_min((float)color.r * (float)s, 255),
		plt_min((float)color.a * (float)s, 255)
	};
}

Plt_Color8 plt_color8_blend(Plt_Color8 a, Plt_Color8 b) {
	float b_mul = (float)b.a / 255.0f;
	float a_mul = 1.0f - b_mul;

	return (Plt_Color8){
		plt_min((float)a.b * a_mul + (float)b.b * b_mul, 255),
		plt_min((float)a.g * a_mul + (float)b.g * b_mul, 255),
		plt_min((float)a.r * a_mul + (float)b.r * b_mul, 255),
		plt_min((float)a.a + (float)b.a, 255)
	};
}