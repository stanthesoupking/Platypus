#pragma once

#include "platypus/platypus.h"

typedef struct Plt_Vertex_Processor Plt_Vertex_Processor;
typedef struct Plt_Vertex_Processor_Result {
	unsigned int vertex_count;

	float *clipspace_x;
	float *clipspace_y;
	float *clipspace_z;

	int *screen_positions_x;
	int *screen_positions_y;

	float *model_uvs_x;
	float *model_uvs_y;

	float *world_normals_x;
	float *world_normals_y;
	float *world_normals_z;
} Plt_Vertex_Processor_Result;

Plt_Vertex_Processor *plt_vertex_processor_create();
void plt_vertex_processor_destroy(Plt_Vertex_Processor **processor);

typedef struct Plt_Mesh Plt_Mesh;
Plt_Vertex_Processor_Result plt_vertex_processor_process_mesh(Plt_Vertex_Processor *processor, Plt_Mesh *mesh, Plt_Vector2i viewport, Plt_Matrix4x4f model, Plt_Matrix4x4f mvp);