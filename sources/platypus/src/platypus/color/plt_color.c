#include "platypus/platypus.h"

inline Plt_Color8 plt_color8_make(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
	return (Plt_Color8) {b, g, r, a};
}

inline Plt_Color8 plt_color8_multiply_scalar(Plt_Color8 color, float s) {
	return (Plt_Color8){color.b * s, color.g * s, color.r * s, color.a * s};
}
