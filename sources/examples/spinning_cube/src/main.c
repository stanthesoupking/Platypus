#include <stdio.h>

#include "platypus/platypus.h"

int main(int argc, char **argv) {
	printf("Hello\n");

	Plt_Application *app = plt_application_create("Platypus - Spinning Cube", 860, 640, Plt_Application_Option_None);
	Plt_Renderer *renderer = plt_application_get_renderer(app);

	Plt_Mesh *triangle = plt_mesh_create(3);
	plt_mesh_set_position(triangle, 0, (Plt_Vector3f){-1, -1, 0});
	plt_mesh_set_position(triangle, 1, (Plt_Vector3f){ 1, -1, 0});
	plt_mesh_set_position(triangle, 2, (Plt_Vector3f){-1,  1, 0});

	while (!plt_application_should_close(app)) {
		plt_application_update(app);
		
		// Render
		plt_renderer_clear(renderer, plt_color8_make(0,0,0,255));
		plt_renderer_set_model_matrix(renderer, plt_matrix_scale_make((Plt_Vector3f){0.5f, 0.5f, 1.0f}));
		plt_renderer_draw_mesh(renderer, triangle);
		plt_renderer_present(renderer);
	}

	plt_mesh_destroy(&triangle);
	plt_application_destroy(&app);

	return 0;
}
