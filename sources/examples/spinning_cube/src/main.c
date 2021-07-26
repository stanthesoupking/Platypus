#include <stdio.h>

#include "platypus/platypus.h"

int main(int argc, char **argv) {
	printf("Hello\n");

	Plt_Application *app = plt_application_create("Platypus - Spinning Cube", 860, 640, 4, Plt_Application_Option_None);
	Plt_Renderer *renderer = plt_application_get_renderer(app);

	Plt_Mesh *triangle = plt_mesh_create(36);
	
	// front
	plt_mesh_set_position(triangle, 0, (Plt_Vector3f){-1, -1, -1});
	plt_mesh_set_position(triangle, 1, (Plt_Vector3f){ 1, -1, -1});
	plt_mesh_set_position(triangle, 2, (Plt_Vector3f){-1,  1, -1});
	plt_mesh_set_position(triangle, 3, (Plt_Vector3f){ 1, -1, -1});
	plt_mesh_set_position(triangle, 4, (Plt_Vector3f){ 1,  1, -1});
	plt_mesh_set_position(triangle, 5, (Plt_Vector3f){-1,  1, -1});
	
	// back
	plt_mesh_set_position(triangle, 6, (Plt_Vector3f){-1, -1,  1});
	plt_mesh_set_position(triangle, 7, (Plt_Vector3f){ 1, -1,  1});
	plt_mesh_set_position(triangle, 8, (Plt_Vector3f){-1,  1,  1});
	plt_mesh_set_position(triangle, 9, (Plt_Vector3f){ 1, -1,  1});
	plt_mesh_set_position(triangle, 10, (Plt_Vector3f){ 1,  1,  1});
	plt_mesh_set_position(triangle, 11, (Plt_Vector3f){-1,  1,  1});
	
	// top
	plt_mesh_set_position(triangle, 12, (Plt_Vector3f){-1,  1, -1});
	plt_mesh_set_position(triangle, 13, (Plt_Vector3f){ 1,  1, -1});
	plt_mesh_set_position(triangle, 14, (Plt_Vector3f){-1,  1,  1});
	plt_mesh_set_position(triangle, 15, (Plt_Vector3f){ 1,  1, -1});
	plt_mesh_set_position(triangle, 16, (Plt_Vector3f){ 1,  1,  1});
	plt_mesh_set_position(triangle, 17, (Plt_Vector3f){-1,  1,  1});
	
	// bottom
	plt_mesh_set_position(triangle, 18, (Plt_Vector3f){-1, -1, -1});
	plt_mesh_set_position(triangle, 19, (Plt_Vector3f){ 1, -1, -1});
	plt_mesh_set_position(triangle, 20, (Plt_Vector3f){-1, -1,  1});
	plt_mesh_set_position(triangle, 21, (Plt_Vector3f){ 1, -1, -1});
	plt_mesh_set_position(triangle, 22, (Plt_Vector3f){ 1, -1,  1});
	plt_mesh_set_position(triangle, 23, (Plt_Vector3f){-1, -1,  1});
	
	// left
	plt_mesh_set_position(triangle, 24, (Plt_Vector3f){-1, -1, -1});
	plt_mesh_set_position(triangle, 25, (Plt_Vector3f){-1,  1, -1});
	plt_mesh_set_position(triangle, 26, (Plt_Vector3f){-1, -1,  1});
	plt_mesh_set_position(triangle, 27, (Plt_Vector3f){-1,  1, -1});
	plt_mesh_set_position(triangle, 28, (Plt_Vector3f){-1,  1,  1});
	plt_mesh_set_position(triangle, 29, (Plt_Vector3f){-1, -1,  1});
	
	// right
	plt_mesh_set_position(triangle, 30, (Plt_Vector3f){ 1, -1, -1});
	plt_mesh_set_position(triangle, 31, (Plt_Vector3f){ 1,  1, -1});
	plt_mesh_set_position(triangle, 32, (Plt_Vector3f){ 1, -1,  1});
	plt_mesh_set_position(triangle, 33, (Plt_Vector3f){ 1,  1, -1});
	plt_mesh_set_position(triangle, 34, (Plt_Vector3f){ 1,  1,  1});
	plt_mesh_set_position(triangle, 35, (Plt_Vector3f){ 1, -1,  1});
	
	float r = 0.0f;

	while (!plt_application_should_close(app)) {
		r += 0.003f;
		
		plt_application_update(app);
		
		// Render
		Plt_Matrix4x4f translate = plt_matrix_translate_make((Plt_Vector3f){0,0.25f,-r * 5.0f - 15.0f});
		Plt_Matrix4x4f rotate = plt_matrix_rotate_make((Plt_Vector3f){r * 0.5,r,0});
		Plt_Matrix4x4f scale = plt_matrix_scale_make((Plt_Vector3f){0.5f, 0.5f, 0.5f});
		Plt_Matrix4x4f model = plt_matrix_multiply(translate, plt_matrix_multiply(rotate, scale));

		Plt_Vector2i size = plt_renderer_get_framebuffer_size(renderer);
		float aspect_ratio = size.x / (float)size.y;
		Plt_Matrix4x4f projection = plt_matrix_perspective_make(aspect_ratio, plt_math_deg2rad(70.0f), 0.1f, 100.0f);
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
