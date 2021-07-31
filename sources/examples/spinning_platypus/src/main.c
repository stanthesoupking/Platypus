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
	Plt_Application *app = plt_application_create("Platypus - Spinning Platypus", 860, 640, 2, Plt_Application_Option_None);
	Plt_Renderer *renderer = plt_application_get_renderer(app);

	Plt_Mesh *platypus_mesh = plt_mesh_load_ply("assets/platypus.ply");
	Plt_Texture *platypus_texture = plt_texture_load("assets/platypus.png");
	
	plt_renderer_bind_texture(renderer, platypus_texture);
	plt_renderer_set_lighting_model(renderer, Plt_Lighting_Model_Vertex_Lit);
	plt_renderer_set_ambient_lighting_color(renderer, plt_color8_make(30, 50, 40, 255));
	plt_renderer_set_directional_lighting_color(renderer, plt_color8_make(255, 200, 200, 255));
	plt_renderer_set_directional_lighting_direction(renderer, (Plt_Vector3f){-0.3f,-1.0f,0.1f});
	
	float r = 0.0f;

	float frame_time_accumulator = 0.0f;
	float average_frame_time = 0.0f;
	unsigned int total_frames = 0;

	float prev_frame = millis();

	while (!plt_application_should_close(app)) {
		float new_frame_time = millis();
		float delta_time = plt_max(new_frame_time - prev_frame, 0);
		prev_frame = new_frame_time;
		r += 0.001f * delta_time;
		
		plt_application_update(app);

		float start = millis();
		
		// Render
		Plt_Matrix4x4f translate = plt_matrix_translate_make((Plt_Vector3f){0,0.1f,-15.0f});
		Plt_Matrix4x4f rotate = plt_matrix_rotate_make((Plt_Vector3f){PLT_PI,r - 3.0,0});
		Plt_Matrix4x4f scale = plt_matrix_scale_make((Plt_Vector3f){0.7, 0.7, 0.7});
		Plt_Matrix4x4f model = plt_matrix_multiply(translate, plt_matrix_multiply(rotate, scale));
		
		Plt_Matrix4x4f camera_translate = plt_matrix_translate_make((Plt_Vector3f){0,5.5f,-10});
		Plt_Matrix4x4f camera_rotate = plt_matrix_rotate_make((Plt_Vector3f){-0.4,0,0});
		plt_renderer_set_view_matrix(renderer, plt_matrix_multiply(camera_translate, camera_rotate));

		Plt_Vector2i size = plt_renderer_get_framebuffer_size(renderer);
		float aspect_ratio = size.x / (float)size.y;
		Plt_Matrix4x4f projection = plt_matrix_perspective_make(aspect_ratio, plt_math_deg2rad(70.0f), 0.1f, 200.0f);
		plt_renderer_set_projection_matrix(renderer, projection);

		plt_renderer_set_primitive_type(renderer, Plt_Primitive_Type_Triangle);
		plt_renderer_set_point_size(renderer, 8);
		
		plt_renderer_clear(renderer, plt_color8_make(35,45,30,255));
		plt_renderer_set_model_matrix(renderer, model);
		plt_renderer_set_primitive_type(renderer, Plt_Primitive_Type_Triangle);
		plt_renderer_draw_mesh(renderer, platypus_mesh);

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

	plt_mesh_destroy(&platypus_mesh);
	plt_texture_destroy(&platypus_texture);
	plt_application_destroy(&app);

	return 0;
}
