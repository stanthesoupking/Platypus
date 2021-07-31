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

	Plt_Mesh *cube = plt_mesh_create_cube((Plt_Vector3f){2,2,2});
	Plt_Texture *crate_texture = plt_texture_load("assets/crate.png");
	// Plt_Mesh *cube = plt_mesh_load_ply("../sources/examples/spinning_cube/assets/teapot3.ply");
	// Plt_Texture *crate_texture = plt_texture_load("../sources/examples/spinning_cube/assets/teapot.png");
	
	plt_renderer_bind_texture(renderer, crate_texture);
	plt_renderer_set_lighting_model(renderer, Plt_Lighting_Model_Vertex_Lit);
	
	float r = 0.0f;

	float frame_time_accumulator = 0.0f;
	float average_frame_time = 0.0f;
	unsigned int total_frames = 0;

	while (!plt_application_should_close(app)) {
		r += 0.01f;
		
		plt_application_update(app);

		float start = millis();
		
		// Render
		Plt_Matrix4x4f translate = plt_matrix_translate_make((Plt_Vector3f){0,0,-r * 0.1f - 12.0f});
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

		float frame_time = plt_max(millis() - start, 0);
		frame_time_accumulator += frame_time;
		++total_frames;

		if (total_frames > 100) {
			average_frame_time = frame_time_accumulator / total_frames;
			frame_time_accumulator = 0.0f;
			total_frames = 0;
			printf("Render time: %.2fms\n", average_frame_time);
		}

		plt_renderer_present(renderer);
	}

	plt_mesh_destroy(&cube);
	plt_texture_destroy(&crate_texture);
	plt_application_destroy(&app);

	return 0;
}
