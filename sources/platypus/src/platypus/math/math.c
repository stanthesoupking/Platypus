#include "platypus/platypus.h"
#include "platypus/base/neon.h"

Plt_Matrix4x4f plt_matrix_identity() {
	return (Plt_Matrix4x4f) {{
		{ 1, 0, 0, 0 },
		{ 0, 1, 0, 0 },
		{ 0, 0, 1, 0 },
		{ 0, 0, 0, 1 }
	}};
}

Plt_Matrix4x4f plt_matrix_multiply(Plt_Matrix4x4f left, Plt_Matrix4x4f right) {
	Plt_Matrix4x4f result = {};

	const float z[] = {0, 0, 0, 0};
	const float32x4_t zero = vld1q_f32(z);

	for (unsigned int y = 0; y < 4; ++y) {
		float32x4_t ra = vld1q_f32(left.rows[y]);
		for (unsigned int x = 0; x < 4; ++x) {
			float col[4] = {
				right.rows[0][x],
				right.rows[1][x],
				right.rows[2][x],
				right.rows[3][x],
			};

			float32x4_t cb = vld1q_f32(col);
			float32_t dot = vaddvq_f32(vmlaq_f32(zero, ra, cb));
			result.rows[y][x] = dot;
		}
	}

	return result;
}
