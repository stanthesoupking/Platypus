#pragma once
#include "platform.h"

#ifdef __aarch64__
#define NEON 1
#include <arm_neon.h>
#else
#define SSE 1
#include <xmmintrin.h>
#endif

// MARK: Float

typedef struct simd_float4 {
	union {
		struct {
			float x, y, z, w;
		};
		
		float v[4];

#ifdef NEON
		float32x4_t neon_v;
#elif SSE
		__m128 sse_v;
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

// MARK: Int

typedef struct simd_int4 {
	union {
		struct {
			int x, y, z, w;
		};
		
		int v[4];

#ifdef NEON
		int32x4_t neon_v;
#elif SSE
		__m128 sse_v;
#endif
	};
} simd_int4;

simd_int4 simd_int4_create(int x, int y, int z, int w);
simd_int4 simd_int4_create_scalar(int v);
simd_int4 simd_int4_load(int *p);

simd_int4 simd_int4_add(simd_int4 a, simd_int4 b);
simd_int4 simd_int4_subtract(simd_int4 a, simd_int4 b);
simd_int4 simd_int4_multiply(simd_int4 a, simd_int4 b);

// MARK: Implementation

#ifdef PLT_PLATFORM_WINDOWS
#define simd_inline __forceinline
#elif PLT_PLATFORM_MACOS
#define simd_inline inline __attribute__((always_inline))
#else
#define simd_inline inline
#endif

simd_inline simd_float4 simd_float4_create(float x, float y, float z, float w) {
	return (simd_float4){x, y, z, w};
}

simd_inline simd_float4 simd_float4_create_scalar(float v) {
	return (simd_float4){v, v, v, v};
}

simd_inline simd_float4 simd_float4_load(float *p) {
	return *((simd_float4 *)p);
}

simd_inline simd_float4 simd_float4_add(simd_float4 a, simd_float4 b) {
	#ifdef NEON
	return (simd_float4){ .neon_v = vaddq_f32(a.neon_v, b.neon_v) };
	#elif SSE
	return (simd_float4) { .sse_v = _mm_add_ps(a.sse_v, b.sse_v) };
	#else
	return (simd_float4){ a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
	#endif
}

simd_inline float simd_float4_add_across(simd_float4 v) {
	#ifdef NEON
	return vaddvq_f32(v.neon_v);
	#else
	return v.x + v.y + v.z + v.w;
	#endif
}

simd_inline simd_float4 simd_float4_multiply_add(simd_float4 a, simd_float4 b, simd_float4 c) {
	#ifdef NEON
	return (simd_float4) { .neon_v = vmlaq_f32(a.neon_v, b.neon_v, c.neon_v) };
	#elif SSE
	// Is there a single SSE instruction for this?
	return (simd_float4) { .sse_v = _mm_add_ps(a.sse_v, _mm_mul_ps(b.sse_v, c.sse_v)) };
	#else
	return (simd_float4){ a.x + b.x * c.x, a.y + b.y * c.y, a.z + b.z * c.z, a.w + b.w * c.w };
	#endif
}

simd_inline simd_int4 simd_int4_create(int x, int y, int z, int w) {
	return (simd_int4){x, y, z, w};
}

simd_inline simd_int4 simd_int4_create_scalar(int v) {
	return (simd_int4){v, v, v, v};
}

simd_inline simd_int4 simd_int4_load(int *p) {
	return *((simd_int4 *)p);
}

simd_inline simd_int4 simd_int4_add(simd_int4 a, simd_int4 b) {
	#ifdef NEON
	return (simd_int4){ .neon_v = vaddq_s32(a.neon_v, b.neon_v) };
	#elif SSE
	// TODO
	#else
	return (simd_int4){ a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
	#endif
}

simd_inline simd_int4 simd_int4_subtract(simd_int4 a, simd_int4 b) {
	#ifdef NEON
	return (simd_int4){ .neon_v = vsubq_s32(a.neon_v, b.neon_v) };
	#elif SSE
	// TODO
	#else
	return (simd_int4){ a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
	#endif
}

simd_inline simd_int4 simd_int4_multiply(simd_int4 a, simd_int4 b) {
	#ifdef NEON
	return (simd_int4){ .neon_v = vmulq_s32(a.neon_v, b.neon_v) };
	#elif SSE
	// TODO
	#else
	return (simd_int4){ a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w };
	#endif
}
