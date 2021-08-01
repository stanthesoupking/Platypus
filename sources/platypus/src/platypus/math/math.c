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
