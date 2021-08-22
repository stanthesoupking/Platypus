#include <stdio.h>

#include "platypus/platypus.h"
#include <math.h>

#define Plt_Object_Type_Spinning_Platypus 1
void _spinning_platypus_update(Plt_Object *object, void *type_data, Plt_Frame_State state) {
	object->transform = plt_transform_rotate(object->transform, plt_quaternion_create_from_euler((Plt_Vector3f){0, 0.0005f * state.delta_time, 0}));
	object->transform.translation.y += sinf(state.application_time * 0.0025f) * 0.005f;
}

#define Plt_Object_Type_FPS_Viewer 2
typedef struct Plt_Object_Type_FPS_Viewer_Data {
	Plt_Font *font;
} Plt_Object_Type_FPS_Viewer_Data;

void _fps_viewer_render_ui(Plt_Object *object, void *type_data, Plt_Frame_State state, Plt_Renderer *renderer) {
	Plt_Object_Type_FPS_Viewer_Data *data = type_data;
	char text[32];
	sprintf(text, "FPS:%d", (int)(1000 / state.delta_time));
	plt_renderer_direct_draw_text(renderer, (Plt_Vector2i){0, 0}, data->font, text);
}

int main(int argc, char **argv) {
	Plt_Application *app = plt_application_create("Platypus - Spinning Platypus", 860, 640, 2, Plt_Application_Option_None);
	Plt_Renderer *renderer = plt_application_get_renderer(app);
	
	// Load assets
	Plt_Font *font = plt_font_load("assets/font_10x16.png");
	Plt_Mesh *platypus_mesh = plt_mesh_load_ply("assets/platypus.ply");
	Plt_Texture *platypus_texture = plt_texture_load("assets/platypus.png");
	Plt_Mesh *gun_mesh = plt_mesh_load_ply("assets/gun.ply");
	Plt_Texture *gun_texture = plt_texture_load("assets/gun.png");
	Plt_Mesh *terrain_mesh = plt_mesh_load_ply("assets/terrain.ply");
	Plt_Texture *lava_texture = plt_texture_load("assets/lava.png");

	// Create world
	const unsigned int type_descriptor_count = 2;
	Plt_Object_Type_Descriptor type_descriptors[2] = {
		{ // Spinning Weapon
			.id = Plt_Object_Type_Spinning_Platypus,
			.data_size = 0,
			.update = _spinning_platypus_update,
			.render_scene = NULL,
			.render_ui = NULL
		},
		{ // FPS Viewer
			.id = Plt_Object_Type_FPS_Viewer,
			.data_size = sizeof(Plt_Object_Type_FPS_Viewer_Data),
			.update = NULL,
			.render_scene = NULL,
			.render_ui = _fps_viewer_render_ui
		}
	};
	
	Plt_World *world = plt_world_create(128, type_descriptors, type_descriptor_count);
	plt_application_set_world(app, world);
	
	// Create world objects
	Plt_Object *fps_viewer = plt_object_create(world, NULL, Plt_Object_Type_FPS_Viewer, "FPS Viewer");
	{
		Plt_Object_Type_FPS_Viewer_Data *fps_viewer_data = fps_viewer->type_data;
		fps_viewer_data->font = font;
	}

	Plt_Object *platypus_object = plt_object_create(world, NULL, Plt_Object_Type_Spinning_Platypus, "Platypus");
	platypus_object->transform.rotation = plt_quaternion_create_from_euler((Plt_Vector3f){ PLT_PI, 0, 0 });
	platypus_object->transform.scale = (Plt_Vector3f){ 0.5f, 0.5f, 0.5f };

	Plt_Object *platypus_mesh_renderer = plt_object_create(world, platypus_object, Plt_Object_Type_Mesh_Renderer, "Platypus Mesh Renderer");
	{
		Plt_Object_Type_Mesh_Renderer_Data *mesh_type_data = platypus_mesh_renderer->type_data;
		mesh_type_data->mesh = platypus_mesh;
		mesh_type_data->texture = platypus_texture;
	}

	Plt_Object *platypus_collider = plt_object_create(world, platypus_object, Plt_Object_Type_Collider, "Platypus Collider");
	platypus_collider->transform.translation.y = -0.1f;
	{
		Plt_Object_Type_Collider_Data *collider_data = platypus_collider->type_data;
		collider_data->shape_type = Plt_Shape_Type_Box;
		collider_data->box_shape.size = (Plt_Vector3f){ 1.0f, 0.7f, 5.5f };
	}
	
	Plt_Object *weapon_object = plt_object_create(world, platypus_object, Plt_Object_Type_Collider, "Weapon");
	weapon_object->transform.translation = (Plt_Vector3f){0.45f, -0.5f, 1.0f};
	weapon_object->transform.rotation = plt_quaternion_create_from_euler((Plt_Vector3f){0, plt_math_deg2rad(-90.0f), plt_math_deg2rad(20.0f)});
	weapon_object->transform.scale = (Plt_Vector3f){0.4f, 0.4f, 0.4f};
	{
		Plt_Object_Type_Collider_Data *collider_data = weapon_object->type_data;
		collider_data->shape_type = Plt_Shape_Type_Box;
		collider_data->box_shape.size = (Plt_Vector3f){ 5.0f, 1.0f, 0.7f };
	}

	Plt_Object *weapon_mesh_renderer = plt_object_create(world, weapon_object, Plt_Object_Type_Mesh_Renderer, "Weapon Mesh Renderer");
	{
		Plt_Object_Type_Mesh_Renderer_Data *mesh_type_data = weapon_mesh_renderer->type_data;
		mesh_type_data->mesh = gun_mesh;
		mesh_type_data->texture = gun_texture;
	}
	
	Plt_Object *terrain_object = plt_object_create(world, NULL, Plt_Object_Type_Mesh_Renderer, "Terrain");
	{
		Plt_Object_Type_Mesh_Renderer_Data *mesh_type_data = terrain_object->type_data;
		mesh_type_data->mesh = terrain_mesh;
		mesh_type_data->texture = lava_texture;
	}
	
	Plt_Object *flying_camera_object = plt_object_create(world, NULL, Plt_Object_Type_Flying_Camera_Controller, "Flying Camera");
	flying_camera_object->transform.translation = (Plt_Vector3f){ 0.0f, -0.5f, 5.0f };
	{
		Plt_Object_Type_Flying_Camera_Controller_Data *flying_camera_type_data = flying_camera_object->type_data;
		flying_camera_type_data->speed = 5.0f;
	}
	
	Plt_Object *camera_object = plt_object_create(world, flying_camera_object, Plt_Object_Type_Camera, "Main Camera");
	Plt_Object_Type_Camera_Data *camera_type_data = camera_object->type_data;
	camera_type_data->fov = plt_math_deg2rad(150.0f);
	camera_type_data->near_z = 0.1f;
	camera_type_data->far_z = 100.0f;

	// Application run loop
	while (!plt_application_should_close(app)) {
		plt_application_update(app);
	}

	// Free memory
	plt_mesh_destroy(&platypus_mesh);
	plt_texture_destroy(&platypus_texture);
	plt_mesh_destroy(&gun_mesh);
	plt_texture_destroy(&gun_texture);
	plt_mesh_destroy(&terrain_mesh);
	plt_texture_destroy(&lava_texture);
	plt_font_destroy(&font);
	plt_application_destroy(&app);
	plt_world_destroy(&world);

	return 0;
}
