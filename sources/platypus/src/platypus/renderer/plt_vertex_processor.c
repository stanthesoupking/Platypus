#include "plt_vertex_processor.h"

#include <stdlib.h>
#include "platypus/mesh/plt_mesh.h"

typedef struct Plt_Vertex_Processor_Working_Buffer {
	unsigned int vertex_capacity;

	float *clipspace_x;
	float *clipspace_y;
	float *clipspace_z;
	float *clipspace_w;

	int *screen_positions_x;
	int *screen_positions_y;

	float *world_normals_x;
	float *world_normals_y;
	float *world_normals_z;
} Plt_Vertex_Processor_Working_Buffer;

typedef struct Plt_Vertex_Processor {
	Plt_Vertex_Processor_Working_Buffer working_buffer;
} Plt_Vertex_Processor;

void plt_vertex_processor_free_working_buffer(Plt_Vertex_Processor *processor);

Plt_Vertex_Processor *plt_vertex_processor_create() {
	Plt_Vertex_Processor *processor = malloc(sizeof(Plt_Vertex_Processor));

	processor->working_buffer = (Plt_Vertex_Processor_Working_Buffer){
		.vertex_capacity = 0,
		.clipspace_x = NULL,
		.clipspace_y = NULL,
		.clipspace_z = NULL,
		.clipspace_w = NULL,
		.screen_positions_x = NULL,
		.screen_positions_y = NULL,
		.world_normals_x = NULL,
		.world_normals_y = NULL,
		.world_normals_z = NULL
	};

	return processor;
}

void plt_vertex_processor_destroy(Plt_Vertex_Processor **processor) {
	plt_vertex_processor_free_working_buffer(*processor);
	free(*processor);
	*processor = NULL;
}

void plt_vertex_processor_free_working_buffer(Plt_Vertex_Processor *processor) {
	if (processor->working_buffer.vertex_capacity == 0) {
		// Nothing to free.
		return;
	}
	free(processor->working_buffer.clipspace_x);
	free(processor->working_buffer.clipspace_y);
	free(processor->working_buffer.clipspace_z);
	free(processor->working_buffer.clipspace_w);
	free(processor->working_buffer.screen_positions_x);
	free(processor->working_buffer.screen_positions_y);
	free(processor->working_buffer.world_normals_x);
	free(processor->working_buffer.world_normals_y);
	free(processor->working_buffer.world_normals_z);
	processor->working_buffer.clipspace_x = NULL;
	processor->working_buffer.clipspace_y = NULL;
	processor->working_buffer.clipspace_z = NULL;
	processor->working_buffer.screen_positions_x = NULL;
	processor->working_buffer.screen_positions_y = NULL;
	processor->working_buffer.world_normals_x = NULL;
	processor->working_buffer.world_normals_y = NULL;
	processor->working_buffer.world_normals_z = NULL;
}

void plt_vertex_processor_resize_working_buffer(Plt_Vertex_Processor *processor, unsigned int capacity) {
	if (processor->working_buffer.vertex_capacity == capacity) {
		return;
	}

	if (processor->working_buffer.vertex_capacity > 0) {
		free(processor->working_buffer.clipspace_x);
		free(processor->working_buffer.clipspace_y);
		free(processor->working_buffer.clipspace_z);
		free(processor->working_buffer.clipspace_w);
		free(processor->working_buffer.screen_positions_x);
		free(processor->working_buffer.screen_positions_y);
		free(processor->working_buffer.world_normals_x);
		free(processor->working_buffer.world_normals_y);
		free(processor->working_buffer.world_normals_z);
	}

	if (capacity > 0) {
		processor->working_buffer.clipspace_x = malloc(sizeof(float) * capacity);
		processor->working_buffer.clipspace_y = malloc(sizeof(float) * capacity);
		processor->working_buffer.clipspace_z = malloc(sizeof(float) * capacity);
		processor->working_buffer.clipspace_w = malloc(sizeof(float) * capacity);
		processor->working_buffer.screen_positions_x = malloc(sizeof(int) * capacity);
		processor->working_buffer.screen_positions_y = malloc(sizeof(int) * capacity);
		processor->working_buffer.world_normals_x = malloc(sizeof(float) * capacity);
		processor->working_buffer.world_normals_y = malloc(sizeof(float) * capacity);
		processor->working_buffer.world_normals_z = malloc(sizeof(float) * capacity);
	} else {
		processor->working_buffer.clipspace_x = NULL;
		processor->working_buffer.clipspace_y = NULL;
		processor->working_buffer.clipspace_z = NULL;
		processor->working_buffer.clipspace_w = NULL;
		processor->working_buffer.screen_positions_x = NULL;
		processor->working_buffer.screen_positions_y = NULL;
		processor->working_buffer.world_normals_x = NULL;
		processor->working_buffer.world_normals_y = NULL;
		processor->working_buffer.world_normals_z = NULL;
	}

	processor->working_buffer.vertex_capacity = capacity;
}

Plt_Vertex_Processor_Result plt_vertex_processor_process_mesh(Plt_Vertex_Processor *processor, Plt_Mesh *mesh, Plt_Vector2i viewport, Plt_Matrix4x4f model, Plt_Matrix4x4f mvp) {
	unsigned int vertex_count = mesh->vertex_count;

	if (processor->working_buffer.vertex_capacity < vertex_count) {
		plt_vertex_processor_resize_working_buffer(processor, vertex_count);
	}

	// Input
	float *model_positions_x = mesh->position_x;
	float *model_positions_y = mesh->position_y;
	float *model_positions_z = mesh->position_z;
	float *model_uvs_x = mesh->uv_x;
	float *model_uvs_y = mesh->uv_y;
	float *model_normals_x = mesh->normal_x;
	float *model_normals_y = mesh->normal_y;
	float *model_normals_z = mesh->normal_z;

	// Output
	float *clipspace_x = processor->working_buffer.clipspace_x;
	float *clipspace_y = processor->working_buffer.clipspace_y;
	float *clipspace_z = processor->working_buffer.clipspace_z;
	float *clipspace_w = processor->working_buffer.clipspace_w;
	int *screen_positions_x = processor->working_buffer.screen_positions_x;
	int *screen_positions_y = processor->working_buffer.screen_positions_y;
	float *world_normals_x = processor->working_buffer.world_normals_x;
	float *world_normals_y = processor->working_buffer.world_normals_y;
	float *world_normals_z = processor->working_buffer.world_normals_z;

	for (unsigned int i = 0; i < vertex_count; ++i) {
		Plt_Vector4f input = { model_positions_x[i], model_positions_y[i], model_positions_z[i], 1.0f };
		Plt_Vector4f clipspace = plt_matrix_multiply_vector4f(mvp, input);
		clipspace_x[i] = clipspace.x;
		clipspace_y[i] = clipspace.y;
		clipspace_z[i] = clipspace.z;
		clipspace_w[i] = clipspace.w;
		
		screen_positions_x[i] = ((clipspace.x / clipspace.w) * 0.5f + 0.5f) * viewport.x;
		screen_positions_y[i] = ((clipspace.y / clipspace.w) * 0.5f + 0.5f) * viewport.y;
		
		Plt_Vector4f input_normal = { model_normals_x[i], model_normals_y[i], model_normals_z[i], 0.0f };
		Plt_Vector4f world_normal = plt_matrix_multiply_vector4f(model, input_normal);
		Plt_Vector3f normalized_world_normal = plt_vector3f_normalize((Plt_Vector3f){world_normal.x, world_normal.y, world_normal.z});
		world_normals_x[i] = normalized_world_normal.x;
		world_normals_y[i] = normalized_world_normal.y;
		world_normals_z[i] = normalized_world_normal.z;
	}

	return (Plt_Vertex_Processor_Result) {
		.vertex_count = vertex_count,
		.clipspace_x = clipspace_x,
		.clipspace_y = clipspace_y,
		.clipspace_z = clipspace_z,
		.clipspace_w = clipspace_w,
		.screen_positions_x = screen_positions_x,
		.screen_positions_y = screen_positions_y,
		.model_uvs_x = model_uvs_x,
		.model_uvs_y = model_uvs_y,
		.world_normals_x = world_normals_x,
		.world_normals_y = world_normals_y,
		.world_normals_z = world_normals_z
	};
}
