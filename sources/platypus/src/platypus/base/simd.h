#pragma once

#ifdef __aarch64__
#define NEON
#include <arm_neon.h>
#else
#define SSE
#endif

typedef struct simd_float4 {
	union {
		struct {
			float x, y, z, w;
		};
		
		float v[4];

		#ifdef NEON
		float32x4_t neon_v;
		#endif
	};
} simd_float4;

simd_float4 simd_float4_create(float x, float y, float z, float w);
simd_float4 simd_float4_create_scalar(float v);
simd_float4 simd_float4_load(float *p);

// a + b
simd_float4 simd_float4_add(simd_float4 a, simd_float4 b);

// a + b * c
simd_float4 simd_float4_multiply_add(simd_float4 a, simd_float4 b, simd_float4 c);

// a[0] + a[1] + a[2] + a[3]
float simd_float4_add_across(simd_float4 v);

// MARK: Implementation

#define simd_inline inline __attribute__((always_inline))

simd_inline simd_float4 simd_float4_create(float x, float y, float z, float w) {
	return (simd_float4){x, y, z, w};
}

simd_inline simd_float4 simd_float4_create_scalar(float v) {
	return (simd_float4){v, v, v, v};
}

simd_inline simd_float4 simd_float4_load(float *p) {
	return (simd_float4) {
		.v = { p[0], p[1], p[2], p[3] }
	};
}

simd_inline simd_float4 simd_float4_add(simd_float4 a, simd_float4 b) {
	#ifdef NEON
	return (simd_float4){ .neon_v = vaddq_f32(a.neon_v, b.neon_v) };
	#elif SSE
	
	#else
	return (simd_float4){ a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
	#endif
}

simd_inline float simd_float4_add_across(simd_float4 v) {
	#ifdef NEON
	return vaddvq_f32(v.neon_v);
	#elif SSE
	
	#else
	return v.x + v.y + v.z + v.w;
	#endif
}

simd_inline simd_float4 simd_float4_multiply_add(simd_float4 a, simd_float4 b, simd_float4 c) {
	#ifdef NEON
	return (simd_float4) { .neon_v = vmlaq_f32(a.neon_v, b.neon_v, c.neon_v) };
	#elif SSE
	
	#else
	return (simd_float4){ a.x + b.x * c.x, a.y + b.y * c.y, a.z + b.z * c.z, a.w + b.w * c.w };
	#endif
}