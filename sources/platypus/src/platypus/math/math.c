#include "platypus/platypus.h"
#include "platypus/base/macros.h"
#include "math.h"

#include "platypus/base/simd.h"

inline Plt_Matrix4x4f plt_matrix_identity() {
	return (Plt_Matrix4x4f) {{
		{ 1, 0, 0, 0 },
		{ 0, 1, 0, 0 },
		{ 0, 0, 1, 0 },
		{ 0, 0, 0, 1 }
	}};
}

inline Plt_Matrix4x4f plt_matrix_zero() {
	return (Plt_Matrix4x4f) {{
		{ 0, 0, 0, 0 },
		{ 0, 0, 0, 0 },
		{ 0, 0, 0, 0 },
		{ 0, 0, 0, 0 }
	}};
}

Plt_Matrix4x4f plt_matrix_multiply(Plt_Matrix4x4f left, Plt_Matrix4x4f right) {
	Plt_Matrix4x4f result = plt_matrix_zero();
	
	const simd_float4 zero = simd_float4_create_scalar(0.0f);
	
	for (unsigned int y = 0; y < 4; ++y) {
		simd_float4 ra = {
			left.columns[0][y],
			left.columns[1][y],
			left.columns[2][y],
			left.columns[3][y]
		};
		
		for (unsigned int x = 0; x < 4; ++x) {
			simd_float4 cb = simd_float4_load(right.columns[x]);
			float dot = simd_float4_add_across(simd_float4_multiply_add(zero, ra, cb));
			result.columns[x][y] = dot;
		}
	}
	
	return result;
}

Plt_Matrix4x4f plt_matrix_invert(Plt_Matrix4x4f m) {
	Plt_Matrix4x4f result;
	
	float t[6];
	float det;
	float a = m.columns[0][0], b = m.columns[0][1], c = m.columns[0][2], d = m.columns[0][3];
	float e = m.columns[1][0], f = m.columns[1][1], g = m.columns[1][2], h = m.columns[1][3];
	float i = m.columns[2][0], j = m.columns[2][1], k = m.columns[2][2], l = m.columns[2][3];
	float mv = m.columns[3][0], n = m.columns[3][1], o = m.columns[3][2], p = m.columns[3][3];
	
	t[0] = k * p - o * l; t[1] = j * p - n * l; t[2] = j * o - n * k;
	t[3] = i * p - mv * l; t[4] = i * o - mv * k; t[5] = i * n - mv * j;
	
	result.columns[0][0] =  f * t[0] - g * t[1] + h * t[2];
	result.columns[1][0] =-(e * t[0] - g * t[3] + h * t[4]);
	result.columns[2][0] =  e * t[1] - f * t[3] + h * t[5];
	result.columns[3][0] =-(e * t[2] - f * t[4] + g * t[5]);
	
	result.columns[0][1] =-(b * t[0] - c * t[1] + d * t[2]);
	result.columns[1][1] =  a * t[0] - c * t[3] + d * t[4];
	result.columns[2][1] =-(a * t[1] - b * t[3] + d * t[5]);
	result.columns[3][1] =  a * t[2] - b * t[4] + c * t[5];
	
	t[0] = g * p - o * h; t[1] = f * p - n * h; t[2] = f * o - n * g;
	t[3] = e * p - mv * h; t[4] = e * o - mv * g; t[5] = e * n - mv * f;
	
	result.columns[0][2] =  b * t[0] - c * t[1] + d * t[2];
	result.columns[1][2] =-(a * t[0] - c * t[3] + d * t[4]);
	result.columns[2][2] =  a * t[1] - b * t[3] + d * t[5];
	result.columns[3][2] =-(a * t[2] - b * t[4] + c * t[5]);
	
	t[0] = g * l - k * h; t[1] = f * l - j * h; t[2] = f * k - j * g;
	t[3] = e * l - i * h; t[4] = e * k - i * g; t[5] = e * j - i * f;
	
	result.columns[0][3] =-(b * t[0] - c * t[1] + d * t[2]);
	result.columns[1][3] =  a * t[0] - c * t[3] + d * t[4];
	result.columns[2][3] =-(a * t[1] - b * t[3] + d * t[5]);
	result.columns[3][3] =  a * t[2] - b * t[4] + c * t[5];
	
	det = 1.0f / (a * result.columns[0][0] + b * result.columns[1][0]
				  + c * result.columns[2][0] + d * result.columns[3][0]);
	
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.columns[i][j] *= det;
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

Plt_Transform plt_transform_invert(Plt_Transform transform) {
	return (Plt_Transform) {
		.translation = plt_vector3f_multiply_scalar(transform.translation, -1.0f),
		.rotation = plt_vector3f_multiply_scalar(transform.rotation, -1.0f),
		.scale = plt_vector3f_multiply_scalar(transform.scale, -1.0f),
	};
}

Plt_Matrix4x4f plt_transform_to_matrix(Plt_Transform transform) {
	Plt_Matrix4x4f translate = plt_matrix_translate_make(transform.translation);
	Plt_Matrix4x4f rotate = plt_matrix_rotate_make(transform.rotation);
	Plt_Matrix4x4f scale = plt_matrix_scale_make(transform.scale);
	return plt_matrix_multiply(plt_matrix_multiply(rotate, scale), translate);
}

Plt_Vector4f plt_matrix_multiply_vector4f(Plt_Matrix4x4f m, Plt_Vector4f v) {
	simd_float4 va = simd_float4_create(v.x, v.y, v.z, v.w);
	
	// TODO: Optimise this
	simd_float4 result = simd_float4_create_scalar(0.0f);
	result = simd_float4_multiply_add(result, simd_float4_load(m.columns[0]), simd_float4_create_scalar(v.x));
	result = simd_float4_multiply_add(result, simd_float4_load(m.columns[1]), simd_float4_create_scalar(v.y));
	result = simd_float4_multiply_add(result, simd_float4_load(m.columns[2]), simd_float4_create_scalar(v.z));
	result = simd_float4_multiply_add(result, simd_float4_load(m.columns[3]), simd_float4_create_scalar(v.w));
	
	return (Plt_Vector4f){result.x, result.y, result.z, result.w};
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

Plt_Vector3f plt_vector3f_subtract(Plt_Vector3f a, Plt_Vector3f b) {
	return (Plt_Vector3f){ a.x - b.x, a.y - b.y, a.z - b.z};
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

Plt_Vector2f plt_vector2f_divide_scalar(Plt_Vector2f a, float b) {
	return (Plt_Vector2f){ a.x / b, a.y / b };
}

Plt_Vector3f plt_vector3f_divide_scalar(Plt_Vector3f a, float b) {
	return (Plt_Vector3f){ a.x / b, a.y / b, a.z / b };
}

Plt_Vector3f plt_vector3f_lerp(Plt_Vector3f a, Plt_Vector3f b, float i) {
	float j = 1.0f - i;
	return (Plt_Vector3f) {
		a.x * i + b.x * j,
		a.y * i + b.y * j,
		a.z * i + b.z * j
	};
}

float plt_math_rad2deg(float rad) {
	return rad / (PLT_PI / 180.0f);
}

float plt_math_deg2rad(float deg) {
	return deg * (PLT_PI / 180.0f);
}
