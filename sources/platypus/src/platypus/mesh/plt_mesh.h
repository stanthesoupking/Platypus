#pragma once

#include "platypus/platypus.h"

typedef struct Plt_Mesh {
	int vertex_count;

	float *position_x;
	float *position_y;
	float *position_z;

	float *normal_x;
	float *normal_y;
	float *normal_z;

	float *uv_x;
	float *uv_y;
} Plt_Mesh;