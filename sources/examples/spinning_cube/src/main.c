#include <stdio.h>

#include "platypus/platypus.h"

int main(int argc, char **argv) {
	printf("Hello\n");

	Plt_Application *app = plt_application_create("Platypus - Spinning Cube", 860, 640, 4, Plt_Application_Option_None);
	Plt_Renderer *renderer = plt_application_get_renderer(app);

	Plt_Mesh *triangle = plt_mesh_create(3);
	plt_mesh_set_position(triangle, 0, (Plt_Vector3f){-0.5, -0.5 , 0});
	plt_mesh_set_position(triangle, 1, (Plt_Vector3f){ 0.5, -0.5, 0});
	plt_mesh_set_position(triangle, 2, (Plt_Vector3f){-0.5,  0.5, 0});
	
	float r = 0.0f;

	while (!plt_application_should_close(app)) {
		r += 0.03f;
		
		plt_application_update(app);
		
		// Render
		Plt_Matrix4x4f translate = plt_matrix_translate_make((Plt_Vector3f){0,0,-r - 10.0f});
		Plt_Matrix4x4f rotate = plt_matrix_rotate_make((Plt_Vector3f){0,0,r});
		Plt_Matrix4x4f scale = plt_matrix_scale_make((Plt_Vector3f){0.5f, 0.5f, 1.0f});
		Plt_Matrix4x4f model = plt_matrix_multiply(translate, plt_matrix_multiply(rotate, scale));

		Plt_Vector2i size = plt_renderer_get_framebuffer_size(renderer);
		float aspect_ratio = size.x / (float)size.y;
		Plt_Matrix4x4f projection = plt_matrix_perspective_make(aspect_ratio, plt_math_deg2rad(60.0f), 0.1f, 100.0f);
		plt_renderer_set_projection_matrix(renderer, projection);

		plt_renderer_set_primitive_type(renderer, Plt_Primitive_Type_Triangle);
		plt_renderer_set_point_size(renderer, 8);
		
		plt_renderer_clear(renderer, plt_color8_make(0,0,0,255));
		plt_renderer_set_model_matrix(renderer, model);
		plt_renderer_set_primitive_type(renderer, Plt_Primitive_Type_Triangle);
		plt_renderer_draw_mesh(renderer, triangle);
		// plt_renderer_set_primitive_type(renderer, Plt_Primitive_Type_Point);
		// plt_renderer_draw_mesh(renderer, triangle);
		plt_renderer_present(renderer);
	}

	plt_mesh_destroy(&triangle);
	plt_application_destroy(&app);

	return 0;
}
