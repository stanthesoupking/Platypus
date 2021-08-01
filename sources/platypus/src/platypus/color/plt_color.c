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
