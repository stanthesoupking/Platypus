#include "platypus/platypus.h"

#include <stdlib.h>

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

Plt_Mesh *plt_mesh_create(int vertex_count) {
	Plt_Mesh *mesh = malloc(sizeof(Plt_Mesh));

	mesh->vertex_count = vertex_count;

	mesh->position_x = malloc(sizeof(float) * vertex_count);
	mesh->position_y = malloc(sizeof(float) * vertex_count);
	mesh->position_z = malloc(sizeof(float) * vertex_count);

	mesh->normal_x = malloc(sizeof(float) * vertex_count);
	mesh->normal_y = malloc(sizeof(float) * vertex_count);
	mesh->normal_z = malloc(sizeof(float) * vertex_count);

	mesh->uv_x = malloc(sizeof(float) * vertex_count);
	mesh->uv_y = malloc(sizeof(float) * vertex_count);

	return mesh;
}

void plt_mesh_destroy(Plt_Mesh **mesh) {
	free((*mesh)->position_x);
	free((*mesh)->position_y);
	free((*mesh)->position_z);
	free((*mesh)->normal_x);
	free((*mesh)->normal_y);
	free((*mesh)->normal_z);
	free((*mesh)->uv_x);
	free((*mesh)->uv_y);

	free(*mesh);
	*mesh = NULL;
}

void plt_mesh_set_position(Plt_Mesh *mesh, int index, Plt_Vector3 position) {
	mesh->position_x[index] = position.x;
	mesh->position_y[index] = position.y;
	mesh->position_z[index] = position.z;
}

Plt_Vector3 plt_mesh_get_position(Plt_Mesh *mesh, int index) {
	return (Plt_Vector3) {
		.x = mesh->position_x[index],
		.y = mesh->position_y[index],
		.z = mesh->position_z[index]
	};
}

void plt_mesh_set_normal(Plt_Mesh *mesh, int index, Plt_Vector3 normal) {
	mesh->normal_x[index] = normal.x;
	mesh->normal_y[index] = normal.y;
	mesh->normal_z[index] = normal.z;
}

Plt_Vector3 plt_mesh_get_normal(Plt_Mesh *mesh, int index) {
	return (Plt_Vector3) {
		.x = mesh->normal_x[index],
		.y = mesh->normal_y[index],
		.z = mesh->normal_z[index]
	};
}

void plt_mesh_set_uv(Plt_Mesh *mesh, int index, Plt_Vector2 uv) {
	mesh->uv_x[index] = uv.x;
	mesh->uv_y[index] = uv.y;
}

Plt_Vector2 plt_mesh_get_uv(Plt_Mesh *mesh, int index) {
	return (Plt_Vector2) {
		.x = mesh->uv_x[index],
		.y = mesh->uv_y[index],
	};
}