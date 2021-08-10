#include <stdio.h>

#include "platypus/platypus.h"

int main(int argc, char **argv) {
	Plt_Application *app = plt_application_create("Platypus - Spinning Platypus", 860, 640, 2, Plt_Application_Option_None);
	Plt_Renderer *renderer = plt_application_get_renderer(app);

	const unsigned int type_descriptor_count = 0;
	Plt_Object_Type_Descriptor type_descriptors[1];
	
	Plt_World *world = plt_world_create(128, type_descriptors, type_descriptor_count);
	plt_application_set_world(app, world);

//	Plt_Mesh *platypus_mesh = plt_mesh_load_ply("../sources/examples/spinning_platypus/assets/platypus.ply");
//	Plt_Texture *platypus_texture = plt_texture_load("../sources/examples/spinning_platypus/assets/platypus.png");
	Plt_Mesh *platypus_mesh = plt_mesh_load_ply("assets/gun.ply");
	Plt_Texture *platypus_texture = plt_texture_load("assets/gun.png");

	Plt_Object *platypus_object = plt_world_create_object(world, NULL, Plt_Object_Type_None, "Platypus");

	Plt_Object *platypus_mesh_renderer = plt_world_create_object(world, platypus_object, Plt_Object_Type_Mesh_Renderer, "Mesh");
	{
		Plt_Object_Type_Mesh_Renderer_Data *mesh_type_data = platypus_mesh_renderer->type_data;
		mesh_type_data->mesh = platypus_mesh;
		mesh_type_data->texture = platypus_texture;
	}

	Plt_Mesh *terrain_mesh = plt_mesh_load_ply("assets/terrain.ply");
	Plt_Texture *lava_texture = plt_texture_load("assets/lava.png");
	Plt_Object *terrain_object = plt_world_create_object(world, NULL, Plt_Object_Type_Mesh_Renderer, "Terrain");
	{
		Plt_Object_Type_Mesh_Renderer_Data *mesh_type_data = terrain_object->type_data;
		mesh_type_data->mesh = terrain_mesh;
		mesh_type_data->texture = lava_texture;
	}
	
	Plt_Object *flying_camera_object = plt_world_create_object(world, NULL, Plt_Object_Type_Flying_Camera_Controller, "Flying Camera");
	flying_camera_object->transform.translation = (Plt_Vector3f){ 0.0f, 0.0f, 80.0f };
	flying_camera_object->transform.rotation = plt_quaternion_create_from_euler((Plt_Vector3f){ 0.0f, 0.0f, 0.0f });
	{
		Plt_Object_Type_Flying_Camera_Controller_Data *flying_camera_type_data = flying_camera_object->type_data;
		flying_camera_type_data->speed = 100.0f;
	}
	
	Plt_Object *camera_object = plt_world_create_object(world, flying_camera_object, Plt_Object_Type_Camera, "Main Camera");
	Plt_Object_Type_Camera_Data *camera_type_data = camera_object->type_data;
	camera_type_data->fov = plt_math_deg2rad(150.0f);
	camera_type_data->near_z = 0.1f;
	camera_type_data->far_z = 100.0f;

	plt_renderer_set_lighting_model(renderer, Plt_Lighting_Model_Vertex_Lit);
	plt_renderer_set_ambient_lighting_color(renderer, plt_color8_make(30, 50, 40, 255));
	plt_renderer_set_directional_lighting_color(renderer, plt_color8_make(255, 200, 200, 255));
	plt_renderer_set_directional_lighting_direction(renderer, (Plt_Vector3f){-0.3f,-1.0f,0.1f});
	
	float r = 0.0f;

	float frame_time_accumulator = 0.0f;
	float average_frame_time = 0.0f;
	unsigned int total_frames = 0;

	float prev_frame = plt_application_get_milliseconds_since_creation(app);

	while (!plt_application_should_close(app)) {
		float new_frame_time = plt_application_get_milliseconds_since_creation(app);
		float delta_time = plt_max(new_frame_time - prev_frame, 0);
		prev_frame = new_frame_time;
		r += 0.0005f * delta_time;
		
		float start = plt_application_get_milliseconds_since_creation(app);
		
		// Render
		platypus_object->transform.translation = (Plt_Vector3f){ 0.0f, 0.0f, 0.0f };
		platypus_object->transform.rotation =  plt_quaternion_create_from_euler((Plt_Vector3f){ PLT_PI, r - 3.0f, 0 });
		platypus_object->transform.scale = (Plt_Vector3f){ 0.5f, 0.5f, 0.5f };

		plt_application_update(app);

		float frame_time = plt_max(plt_application_get_milliseconds_since_creation(app) - start, 0);
		frame_time_accumulator += frame_time;
		++total_frames;

		if (total_frames > 100) {
			average_frame_time = frame_time_accumulator / total_frames;
			frame_time_accumulator = 0.0f;
			total_frames = 0;
			printf("Render time: %.2fms\n", average_frame_time);
		}
	}

	plt_mesh_destroy(&platypus_mesh);
	plt_texture_destroy(&platypus_texture);
	plt_application_destroy(&app);
	plt_world_destroy(&world);

	return 0;
}
