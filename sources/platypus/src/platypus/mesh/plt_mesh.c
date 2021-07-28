#include "plt_mesh.h"

#include <stdlib.h>

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

Plt_Mesh *plt_mesh_create_cube(Plt_Vector3f size) {
	Plt_Mesh *cube = plt_mesh_create(36);

	Plt_Vector3f hsize = plt_vector3f_multiply_scalar(size, 0.5f);
	
	// front
	plt_mesh_set_position(cube, 0, (Plt_Vector3f){-hsize.x, -hsize.y, -hsize.z});
	plt_mesh_set_position(cube, 1, (Plt_Vector3f){ hsize.x, -hsize.y, -hsize.z});
	plt_mesh_set_position(cube, 2, (Plt_Vector3f){-hsize.x,  hsize.y, -hsize.z});
	plt_mesh_set_position(cube, 3, (Plt_Vector3f){ hsize.x, -hsize.y, -hsize.z});
	plt_mesh_set_position(cube, 4, (Plt_Vector3f){ hsize.x,  hsize.y, -hsize.z});
	plt_mesh_set_position(cube, 5, (Plt_Vector3f){-hsize.x,  hsize.y, -hsize.z});

	plt_mesh_set_uv(cube, 0, (Plt_Vector2f){ 0.0f, 0.0f });
	plt_mesh_set_uv(cube, 1, (Plt_Vector2f){ 1.0f, 0.0f });
	plt_mesh_set_uv(cube, 2, (Plt_Vector2f){ 0.0f, 1.0f });
	plt_mesh_set_uv(cube, 3, (Plt_Vector2f){ 1.0f, 0.0f });
	plt_mesh_set_uv(cube, 4, (Plt_Vector2f){ 1.0f, 1.0f });
	plt_mesh_set_uv(cube, 5, (Plt_Vector2f){ 0.0f, 1.0f });
	
	plt_mesh_set_normal(cube, 0, (Plt_Vector3f){0, 0, -1});
	plt_mesh_set_normal(cube, 1, (Plt_Vector3f){0, 0, -1});
	plt_mesh_set_normal(cube, 2, (Plt_Vector3f){0, 0, -1});
	plt_mesh_set_normal(cube, 3, (Plt_Vector3f){0, 0, -1});
	plt_mesh_set_normal(cube, 4, (Plt_Vector3f){0, 0, -1});
	plt_mesh_set_normal(cube, 5, (Plt_Vector3f){0, 0, -1});
	
	// back
	plt_mesh_set_position(cube, 6, (Plt_Vector3f){-hsize.x,  hsize.y,  hsize.z});
	plt_mesh_set_position(cube, 7, (Plt_Vector3f){ hsize.x, -hsize.y,  hsize.z});
	plt_mesh_set_position(cube, 8, (Plt_Vector3f){-hsize.x, -hsize.y,  hsize.z});
	plt_mesh_set_position(cube, 9, (Plt_Vector3f){-hsize.x,  hsize.y,  hsize.z});
	plt_mesh_set_position(cube, 10, (Plt_Vector3f){ hsize.x,  hsize.y,  hsize.z});
	plt_mesh_set_position(cube, 11, (Plt_Vector3f){ hsize.x, -hsize.y,  hsize.z});

	plt_mesh_set_uv(cube, 6, (Plt_Vector2f){ 0.0f, 1.0f });
	plt_mesh_set_uv(cube, 7, (Plt_Vector2f){ 1.0f, 0.0f });
	plt_mesh_set_uv(cube, 8, (Plt_Vector2f){ 0.0f, 0.0f });
	plt_mesh_set_uv(cube, 9, (Plt_Vector2f){ 0.0f, 1.0f });
	plt_mesh_set_uv(cube, 10, (Plt_Vector2f){ 1.0f, 1.0f });
	plt_mesh_set_uv(cube, 11, (Plt_Vector2f){ 1.0f, 0.0f });
	
	plt_mesh_set_normal(cube, 6, (Plt_Vector3f){0, 0, 1});
	plt_mesh_set_normal(cube, 7, (Plt_Vector3f){0, 0, 1});
	plt_mesh_set_normal(cube, 8, (Plt_Vector3f){0, 0, 1});
	plt_mesh_set_normal(cube, 9, (Plt_Vector3f){0, 0, 1});
	plt_mesh_set_normal(cube, 10, (Plt_Vector3f){0, 0, 1});
	plt_mesh_set_normal(cube, 11, (Plt_Vector3f){0, 0, 1});
	
	// top
	plt_mesh_set_position(cube, 12, (Plt_Vector3f){-hsize.x,  hsize.y, -hsize.z});
	plt_mesh_set_position(cube, 13, (Plt_Vector3f){ hsize.x,  hsize.y, -hsize.z});
	plt_mesh_set_position(cube, 14, (Plt_Vector3f){-hsize.x,  hsize.y,  hsize.z});
	plt_mesh_set_position(cube, 15, (Plt_Vector3f){ hsize.x,  hsize.y, -hsize.z});
	plt_mesh_set_position(cube, 16, (Plt_Vector3f){ hsize.x,  hsize.y,  hsize.z});
	plt_mesh_set_position(cube, 17, (Plt_Vector3f){-hsize.x,  hsize.y,  hsize.z});

	plt_mesh_set_uv(cube, 12, (Plt_Vector2f){ 0.0f, 0.0f });
	plt_mesh_set_uv(cube, 13, (Plt_Vector2f){ 1.0f, 0.0f });
	plt_mesh_set_uv(cube, 14, (Plt_Vector2f){ 0.0f, 1.0f });
	plt_mesh_set_uv(cube, 15, (Plt_Vector2f){ 1.0f, 0.0f });
	plt_mesh_set_uv(cube, 16, (Plt_Vector2f){ 1.0f, 1.0f });
	plt_mesh_set_uv(cube, 17, (Plt_Vector2f){ 0.0f, 1.0f });
	
	plt_mesh_set_normal(cube, 12, (Plt_Vector3f){0, 1, 0});
	plt_mesh_set_normal(cube, 13, (Plt_Vector3f){0, 1, 0});
	plt_mesh_set_normal(cube, 14, (Plt_Vector3f){0, 1, 0});
	plt_mesh_set_normal(cube, 15, (Plt_Vector3f){0, 1, 0});
	plt_mesh_set_normal(cube, 16, (Plt_Vector3f){0, 1, 0});
	plt_mesh_set_normal(cube, 17, (Plt_Vector3f){0, 1, 0});
	
	// bottom
	plt_mesh_set_position(cube, 20, (Plt_Vector3f){-hsize.x, -hsize.y, -hsize.z});
	plt_mesh_set_position(cube, 19, (Plt_Vector3f){ hsize.x, -hsize.y, -hsize.z});
	plt_mesh_set_position(cube, 18, (Plt_Vector3f){-hsize.x, -hsize.y,  hsize.z});
	plt_mesh_set_position(cube, 23, (Plt_Vector3f){ hsize.x, -hsize.y, -hsize.z});
	plt_mesh_set_position(cube, 22, (Plt_Vector3f){ hsize.x, -hsize.y,  hsize.z});
	plt_mesh_set_position(cube, 21, (Plt_Vector3f){-hsize.x, -hsize.y,  hsize.z});

	plt_mesh_set_uv(cube, 18, (Plt_Vector2f){ 0.0f, 1.0f });
	plt_mesh_set_uv(cube, 19, (Plt_Vector2f){ 1.0f, 0.0f });
	plt_mesh_set_uv(cube, 20, (Plt_Vector2f){ 0.0f, 0.0f });
	plt_mesh_set_uv(cube, 21, (Plt_Vector2f){ 0.0f, 1.0f });
	plt_mesh_set_uv(cube, 22, (Plt_Vector2f){ 1.0f, 1.0f });
	plt_mesh_set_uv(cube, 23, (Plt_Vector2f){ 1.0f, 0.0f });
	
	plt_mesh_set_normal(cube, 18, (Plt_Vector3f){0, -1, 0});
	plt_mesh_set_normal(cube, 19, (Plt_Vector3f){0, -1, 0});
	plt_mesh_set_normal(cube, 20, (Plt_Vector3f){0, -1, 0});
	plt_mesh_set_normal(cube, 21, (Plt_Vector3f){0, -1, 0});
	plt_mesh_set_normal(cube, 22, (Plt_Vector3f){0, -1, 0});
	plt_mesh_set_normal(cube, 23, (Plt_Vector3f){0, -1, 0});
	
	// left
	plt_mesh_set_position(cube, 24, (Plt_Vector3f){-hsize.x, -hsize.y, -hsize.z});
	plt_mesh_set_position(cube, 25, (Plt_Vector3f){-hsize.x,  hsize.y, -hsize.z});
	plt_mesh_set_position(cube, 26, (Plt_Vector3f){-hsize.x, -hsize.y,  hsize.z});
	plt_mesh_set_position(cube, 27, (Plt_Vector3f){-hsize.x,  hsize.y, -hsize.z});
	plt_mesh_set_position(cube, 28, (Plt_Vector3f){-hsize.x,  hsize.y,  hsize.z});
	plt_mesh_set_position(cube, 29, (Plt_Vector3f){-hsize.x, -hsize.y,  hsize.z});

	plt_mesh_set_uv(cube, 24, (Plt_Vector2f){ 0.0f, 0.0f });
	plt_mesh_set_uv(cube, 25, (Plt_Vector2f){ 1.0f, 0.0f });
	plt_mesh_set_uv(cube, 26, (Plt_Vector2f){ 0.0f, 1.0f });
	plt_mesh_set_uv(cube, 27, (Plt_Vector2f){ 1.0f, 0.0f });
	plt_mesh_set_uv(cube, 28, (Plt_Vector2f){ 1.0f, 1.0f });
	plt_mesh_set_uv(cube, 29, (Plt_Vector2f){ 0.0f, 1.0f });
	
	plt_mesh_set_normal(cube, 24, (Plt_Vector3f){-1, 0, 0});
	plt_mesh_set_normal(cube, 25, (Plt_Vector3f){-1, 0, 0});
	plt_mesh_set_normal(cube, 26, (Plt_Vector3f){-1, 0, 0});
	plt_mesh_set_normal(cube, 27, (Plt_Vector3f){-1, 0, 0});
	plt_mesh_set_normal(cube, 28, (Plt_Vector3f){-1, 0, 0});
	plt_mesh_set_normal(cube, 29, (Plt_Vector3f){-1, 0, 0});
	
	// right
	plt_mesh_set_position(cube, 32, (Plt_Vector3f){ hsize.x, -hsize.y, -hsize.z});
	plt_mesh_set_position(cube, 31, (Plt_Vector3f){ hsize.x,  hsize.y, -hsize.z});
	plt_mesh_set_position(cube, 30, (Plt_Vector3f){ hsize.x, -hsize.y,  hsize.z});
	plt_mesh_set_position(cube, 35, (Plt_Vector3f){ hsize.x,  hsize.y, -hsize.z});
	plt_mesh_set_position(cube, 34, (Plt_Vector3f){ hsize.x,  hsize.y,  hsize.z});
	plt_mesh_set_position(cube, 33, (Plt_Vector3f){ hsize.x, -hsize.y,  hsize.z});

	plt_mesh_set_uv(cube, 30, (Plt_Vector2f){ 0.0f, 1.0f });
	plt_mesh_set_uv(cube, 31, (Plt_Vector2f){ 1.0f, 0.0f });
	plt_mesh_set_uv(cube, 32, (Plt_Vector2f){ 0.0f, 0.0f });
	plt_mesh_set_uv(cube, 33, (Plt_Vector2f){ 0.0f, 1.0f });
	plt_mesh_set_uv(cube, 34, (Plt_Vector2f){ 1.0f, 1.0f });
	plt_mesh_set_uv(cube, 35, (Plt_Vector2f){ 1.0f, 0.0f });
	
	plt_mesh_set_normal(cube, 30, (Plt_Vector3f){1, 0, 0});
	plt_mesh_set_normal(cube, 31, (Plt_Vector3f){1, 0, 0});
	plt_mesh_set_normal(cube, 32, (Plt_Vector3f){1, 0, 0});
	plt_mesh_set_normal(cube, 33, (Plt_Vector3f){1, 0, 0});
	plt_mesh_set_normal(cube, 34, (Plt_Vector3f){1, 0, 0});
	plt_mesh_set_normal(cube, 35, (Plt_Vector3f){1, 0, 0});

	return cube;
}

void plt_mesh_set_position(Plt_Mesh *mesh, int index, Plt_Vector3f position) {
	mesh->position_x[index] = position.x;
	mesh->position_y[index] = position.y;
	mesh->position_z[index] = position.z;
}

Plt_Vector3f plt_mesh_get_position(Plt_Mesh *mesh, int index) {
	return (Plt_Vector3f) {
		.x = mesh->position_x[index],
		.y = mesh->position_y[index],
		.z = mesh->position_z[index]
	};
}

void plt_mesh_set_normal(Plt_Mesh *mesh, int index, Plt_Vector3f normal) {
	mesh->normal_x[index] = normal.x;
	mesh->normal_y[index] = normal.y;
	mesh->normal_z[index] = normal.z;
}

Plt_Vector3f plt_mesh_get_normal(Plt_Mesh *mesh, int index) {
	return (Plt_Vector3f) {
		.x = mesh->normal_x[index],
		.y = mesh->normal_y[index],
		.z = mesh->normal_z[index]
	};
}

void plt_mesh_set_uv(Plt_Mesh *mesh, int index, Plt_Vector2f uv) {
	mesh->uv_x[index] = uv.x;
	mesh->uv_y[index] = uv.y;
}

Plt_Vector2f plt_mesh_get_uv(Plt_Mesh *mesh, int index) {
	return (Plt_Vector2f) {
		.x = mesh->uv_x[index],
		.y = mesh->uv_y[index],
	};
}
