#include <stdio.h>

#include "platypus/platypus.h"

#include <unistd.h>
#include <sys/time.h>

float millis(void) {
	struct timespec curTime;
	clock_gettime(_CLOCK_REALTIME, &curTime);
	return curTime.tv_nsec / 1000000.0f;
}


int main(int argc, char **argv) {
	printf("Hello\n");

	Plt_Application *app = plt_application_create("Platypus - Spinning Cube", 860, 640, 1, Plt_Application_Option_None);
	Plt_Renderer *renderer = plt_application_get_renderer(app);

	Plt_Mesh *cube = plt_mesh_load_ply("../sources/examples/spinning_cube/assets/teapot2.ply");//plt_mesh_create_cube((Plt_Vector3f){1,1,1});
	Plt_Texture *crate_texture = plt_texture_load("../sources/examples/spinning_cube/assets/teapot.png");
	
	plt_renderer_bind_texture(renderer, crate_texture);
	
	float r = 0.0f;

	float average_frame_time = 0.0f;
	unsigned int total_frames = 0;

	while (!plt_application_should_close(app)) {
		r += 0.001f;
		
		plt_application_update(app);

		float start = millis();
		
		// Render
		Plt_Matrix4x4f translate = plt_matrix_translate_make((Plt_Vector3f){0,0,-r * 1.0f - 6.0f});
		Plt_Matrix4x4f rotate = plt_matrix_rotate_make((Plt_Vector3f){r * 0.5,r - 3.0,r * 0.25 + 3.0});
		Plt_Matrix4x4f scale = plt_matrix_scale_make((Plt_Vector3f){0.4, 0.4, 0.4});
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
		plt_renderer_draw_mesh(renderer, cube);
		// plt_renderer_set_primitive_type(renderer, Plt_Primitive_Type_Point);
		// plt_renderer_draw_mesh(renderer, triangle);
		plt_renderer_present(renderer);

		float frame_time = plt_max(millis() - start, 0);
		average_frame_time += frame_time;
		++total_frames;
		printf("Frame time: %.2fms\n", average_frame_time / total_frames);
	}

	plt_mesh_destroy(&cube);
	plt_texture_destroy(&crate_texture);
	plt_application_destroy(&app);

	return 0;
}
