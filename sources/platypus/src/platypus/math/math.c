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

Plt_Matrix4x4f plt_matrix_perspective_make(float aspect_ratio, float fov, float near_z, float far_z) {
	float ys = 1 / tanf(fov * 0.5);
	float xs = ys / aspect_ratio;
	float zs = far_z / (near_z - far_z);

	return (Plt_Matrix4x4f) {{
		{xs, 0, 0, 0},
		{0, ys, 0, 0},
		{0, 0, zs, near_z * zs},
		{0, 0, -1, 0}
	}};
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

float plt_vector2f_dot_product(Plt_Vector2f a, Plt_Vector2f b) {
	return a.x * b.y + a.y * b.x;
}

float plt_vector3f_dot_product(Plt_Vector3f a, Plt_Vector3f b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

Plt_Vector3f plt_vector3f_normalize(Plt_Vector3f v) {
	float magnitude = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	return (Plt_Vector3f) {
		v.x / magnitude,
		v.y / magnitude,
		v.z / magnitude
	};
}

Plt_Vector2f plt_vector2f_add(Plt_Vector2f a, Plt_Vector2f b) {
	return (Plt_Vector2f){ a.x + b.x, a.y + b.y};
}

Plt_Vector3f plt_vector3f_add(Plt_Vector3f a, Plt_Vector3f b) {
	return (Plt_Vector3f){ a.x + b.x, a.y + b.y, a.z + b.z};
}

Plt_Vector4f plt_vector4f_add(Plt_Vector4f a, Plt_Vector4f b) {
	return (Plt_Vector4f){ a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
}

Plt_Vector2f plt_vector2f_multiply_scalar(Plt_Vector2f a, float b) {
	return (Plt_Vector2f){ a.x * b, a.y * b };
}

Plt_Vector3f plt_vector3f_multiply_scalar(Plt_Vector3f a, float b) {
	return (Plt_Vector3f){ a.x * b, a.y * b, a.z * b };
}

Plt_Vector4f plt_vector4f_multiply_scalar(Plt_Vector4f a, float b) {
	return (Plt_Vector4f){ a.x * b, a.y * b, a.z * b, a.w * b };
}

float plt_math_rad2deg(float rad) {
	return rad / (PLT_PI / 180.0f);
}

float plt_math_deg2rad(float deg) {
	return deg * (PLT_PI / 180.0f);
}
