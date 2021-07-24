#include "platypus/platypus.h"
#include "platypus/base/neon.h"
#include "platypus/base/macros.h"

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

	const float32x4_t zero = {};

	for (unsigned int y = 0; y < 4; ++y) {
		float row[4] = {
			left.columns[0][y],
			left.columns[1][y],
			left.columns[2][y],
			left.columns[3][y]
		};
		float32x4_t ra = vld1q_f32(row);

		for (unsigned int x = 0; x < 4; ++x) {
			float32x4_t cb = vld1q_f32(right.columns[x]);
			float32_t dot = vaddvq_f32(vmlaq_f32(zero, ra, cb));
			result.columns[x][y] = dot;
		}
	}

	return result;
}

Plt_Matrix4x4f plt_matrix_scale_make(Plt_Vector3f scale) {
	return (Plt_Matrix4x4f) {{
		{ scale.x, 0, 0, 0 },
		{ 0, scale.y, 0, 0 },
		{ 0, 0, scale.z, 0 },
		{ 0, 0, 0, 1 }
	}};
}

Plt_Vector4f plt_matrix_multiply_vector4f(Plt_Matrix4x4f m, Plt_Vector4f v) {
	const float32x4_t zero = {};

	float32x4_t va = plt_cast(float32x4_t, v);

	for (unsigned int y = 0; y < 4; ++y) {
		float32x4_t cb = vld1q_f32(m.columns[y]);
		float32_t dot = vaddvq_f32(vmlaq_f32(zero, va, cb));
		va[y] = dot;
	}

	return plt_cast(Plt_Vector4f, va);
}
