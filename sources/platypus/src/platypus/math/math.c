#include "platypus/platypus.h"
#include "platypus/base/neon.h"
#include "platypus/base/macros.h"
#include "math.h"

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

Plt_Matrix4x4f plt_matrix_translate_make(Plt_Vector3f translate) {
	return (Plt_Matrix4x4f) {{
		{ 1, 0, 0, 0 },
		{ 0, 1, 0, 0 },
		{ 0, 0, 1, 0 },
		{ translate.x, translate.y, translate.z, 1 }
	}};
}

Plt_Matrix4x4f plt_matrix_scale_make(Plt_Vector3f scale) {
	return (Plt_Matrix4x4f) {{
		{ scale.x, 0, 0, 0 },
		{ 0, scale.y, 0, 0 },
		{ 0, 0, scale.z, 0 },
		{ 0, 0, 0, 1 }
	}};
}

Plt_Matrix4x4f plt_matrix_rotate_make(Plt_Vector3f rotate) {
	Plt_Matrix4x4f rx = {{
		{ 1, 0, 0, 0 },
		{ 0, cosf(rotate.x), sinf(rotate.x), 0 },
		{ 0, -sinf(rotate.x), cosf(rotate.x), 0 },
		{ 0, 0, 0, 1 }
	}};

	Plt_Matrix4x4f ry = {{
		{ cosf(rotate.y), 0, -sinf(rotate.y), 0 },
		{ 0, 1, 0, 0 },
		{ sinf(rotate.y), 0, cosf(rotate.y), 0 },
		{0, 0, 0, 1}
	}};

	Plt_Matrix4x4f rz = {{
		{ cosf(rotate.z), -sinf(rotate.z), 0, 0 },
		{ sinf(rotate.z), cosf(rotate.z), 0, 0 },
		{ 0, 0, 1, 0 },
		{ 0, 0, 0, 1 }
	}};
	
	return plt_matrix_multiply(plt_matrix_multiply(rz, ry), rx);
}

Plt_Vector4f plt_matrix_multiply_vector4f(Plt_Matrix4x4f m, Plt_Vector4f v) {
	float32x4_t va = {v.x, v.y, v.z, v.w};
	
	// TODO: Optimise this
	float32x4_t result = {};
	result = vmlaq_f32(result, vld1q_f32(m.columns[0]), (float32x4_t){v.x,v.x,v.x,v.x});
	result = vmlaq_f32(result, vld1q_f32(m.columns[1]), (float32x4_t){v.y,v.y,v.y,v.y});
	result = vmlaq_f32(result, vld1q_f32(m.columns[2]), (float32x4_t){v.z,v.z,v.z,v.z});
	result = vmlaq_f32(result, vld1q_f32(m.columns[3]), (float32x4_t){v.w,v.w,v.w,v.w});

	return (Plt_Vector4f){result[0], result[1], result[2], result[3]};
}
