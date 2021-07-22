#include <stdio.h>

#include "platypus/platypus.h"

int main(int argc, char **argv) {
	printf("Hello\n");

	Plt_Application *app = plt_application_create("Platypus - Spinning Cube", 860, 640, Plt_Application_Option_None);

	Plt_Mesh *triangle = plt_mesh_create(3);
	plt_mesh_set_position(triangle, 0, (Plt_Vector3){-0.5f, -0.5f, 0.0f});
	plt_mesh_set_position(triangle, 1, (Plt_Vector3){ 0.5f, -0.5f, 0.0f});
	plt_mesh_set_position(triangle, 2, (Plt_Vector3){-0.5f,  0.5f, 0.0f});

	while (!plt_application_should_close(app)) {
		plt_application_update(app);
	}

	plt_mesh_destroy(&triangle);
	plt_application_destroy(&app);

	return 0;
}