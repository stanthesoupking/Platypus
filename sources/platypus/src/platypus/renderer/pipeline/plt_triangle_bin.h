#pragma once

#include "platypus/platypus.h"
#include "platypus/base/plt_simd.h"

#define PLT_TRIANGLE_BIN_MAX_TRIANGLES 1024

// Framebuffer pixel data = 16 x 16 x 4 = 1024 bytes
// Changing this will require updating various unwrapped loops (the compiler will complain for all these locations)
#define PLT_TRIANGLE_BIN_SIZE 16

typedef enum Plt_Triangle_Tile_Coverage {
	// Triangle is not visibile the tile
	Plt_Triangle_Tile_Coverage_None,

	// Triangle is visibile the tile but some areas are partially covered
	Plt_Triangle_Tile_Coverage_Partial,

	// Triangle covers the entire tile
	Plt_Triangle_Tile_Coverage_Full
} Plt_Triangle_Tile_Coverage;

typedef struct Plt_Triangle_Bin_Data_Buffer {
	unsigned int triangle_count;
	
	// Barycentric coordinates
	simd_int4 *bc_initial;
	simd_int4 *bc_increment_x;
	simd_int4 *bc_increment_y;
	float *triangle_area;
	
	// Depth
	float *depth0;
	float *depth1;
	float *depth2;
	
	// Texture coordinates
	Plt_Vector2f *uv0;
	Plt_Vector2f *uv1;
	Plt_Vector2f *uv2;
	
	// Lighting
	Plt_Vector3f *lighting0;
	Plt_Vector3f *lighting1;
	Plt_Vector3f *lighting2;
} Plt_Triangle_Bin_Data_Buffer;

typedef struct Plt_Triangle_Bin_Entry {
	unsigned int index;
	Plt_Triangle_Tile_Coverage coverage;
	Plt_Triangle_Bin_Data_Buffer *buffer;
	Plt_Texture *texture;
} Plt_Triangle_Bin_Entry;

typedef struct Plt_Triangle_Bin {
	unsigned int triangle_count;
	Plt_Triangle_Bin_Entry entries[PLT_TRIANGLE_BIN_MAX_TRIANGLES];
} Plt_Triangle_Bin;
