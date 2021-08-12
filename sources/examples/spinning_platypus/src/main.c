#include <stdio.h>

#include "platypus/platypus.h"
#include <math.h>

#define Plt_Object_Type_Spinning_Weapon 1
void _spinning_weapon_update(Plt_Object *object, void *type_data, Plt_Frame_State state) {
	object->transform = plt_transform_rotate(object->transform, plt_quaternion_create_from_euler((Plt_Vector3f){0, 0.0005f * state.delta_time, 0}));
	object->transform.translation.y += sinf(state.application_time * 0.0025f) * 0.005f;

	// Handle collisions
	unsigned int collision_count = 0;
	Plt_Object **collisions = plt_object_get_collisions(object, &collision_count);
	for (unsigned int i = 0; i < collision_count; ++i) {
		printf("'%s' collided with '%s'!\n", object->name, collisions[i]->name);
	}
	
	if (collision_count == 0) {
		printf("'%s' didn't collide with anything.\n", object->name);
	}
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

	const unsigned int type_descriptor_count = 2;
	Plt_Object_Type_Descriptor type_descriptors[2] = {
		{ // Spinning Weapon
			.id = Plt_Object_Type_Spinning_Weapon,
			.data_size = 0,
			.update = _spinning_weapon_update,
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

	Plt_Font *font = plt_font_load("assets/font_10x16.png");
	
	Plt_Object *fps_viewer = plt_world_create_object(world, NULL, Plt_Object_Type_FPS_Viewer, "FPS Viewer");
	{
		Plt_Object_Type_FPS_Viewer_Data *fps_viewer_data = fps_viewer->type_data;
		fps_viewer_data->font = font;
	}

	Plt_Mesh *platypus_mesh = plt_mesh_load_ply("assets/gun.ply");
	Plt_Texture *platypus_texture = plt_texture_load("assets/gun.png");
	
	Plt_Mesh *crate_mesh = plt_mesh_create_cube((Plt_Vector3f){1, 1, 1});
	Plt_Texture *crate_texture = plt_texture_load("assets/crate.png");

	Plt_Object *weapon_object = plt_world_create_object(world, NULL, Plt_Object_Type_Spinning_Weapon, "Weapon");
	weapon_object->transform.rotation = plt_quaternion_create_from_euler((Plt_Vector3f){ PLT_PI, 0, 0 });
	weapon_object->transform.scale = (Plt_Vector3f){ 0.5f, 0.5f, 0.5f };

	Plt_Object *platypus_mesh_renderer = plt_world_create_object(world, weapon_object, Plt_Object_Type_Mesh_Renderer, "Weapon Mesh Renderer");
	{
		Plt_Object_Type_Mesh_Renderer_Data *mesh_type_data = platypus_mesh_renderer->type_data;
		mesh_type_data->mesh = platypus_mesh;
		mesh_type_data->texture = platypus_texture;
	}

	Plt_Object *weapon_collider = plt_world_create_object(world, weapon_object, Plt_Object_Type_Collider, "Weapon Collider");
	{
		Plt_Object_Type_Collider_Data *collider_data = weapon_collider->type_data;
		collider_data->shape_type = Plt_Shape_Type_Box;
		collider_data->box_shape.size = (Plt_Vector3f){ 5.0f, 1.4f, 1.0f };
	}
	
	Plt_Object *test_box = plt_world_create_object(world, NULL, Plt_Object_Type_Collider, "Test Box Collider");
	test_box->transform.translation = (Plt_Vector3f){1.0f, 0.5f, 0.0f};
	{
		Plt_Object_Type_Collider_Data *collider_data = test_box->type_data;
		collider_data->shape_type = Plt_Shape_Type_Box;
		collider_data->box_shape.size = (Plt_Vector3f){ 1.0f, 1.0f, 1.0f };
	}

	Plt_Object *test_box_renderer = plt_world_create_object(world, test_box, Plt_Object_Type_Mesh_Renderer, "Test Box Renderer");
	{
		Plt_Object_Type_Mesh_Renderer_Data *mesh_type_data = test_box_renderer->type_data;
		mesh_type_data->mesh = crate_mesh;
		mesh_type_data->texture = crate_texture;
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
	flying_camera_object->transform.translation = (Plt_Vector3f){ 0.0f, -1.0f, 10.0f };
	{
		Plt_Object_Type_Flying_Camera_Controller_Data *flying_camera_type_data = flying_camera_object->type_data;
		flying_camera_type_data->speed = 5.0f;
	}
	
	Plt_Object *camera_object = plt_world_create_object(world, flying_camera_object, Plt_Object_Type_Camera, "Main Camera");
	Plt_Object_Type_Camera_Data *camera_type_data = camera_object->type_data;
	camera_type_data->fov = plt_math_deg2rad(150.0f);
	camera_type_data->near_z = 0.1f;
	camera_type_data->far_z = 100.0f;

	plt_renderer_set_lighting_model(renderer, Plt_Lighting_Model_Vertex_Lit);
	plt_renderer_set_ambient_lighting(renderer, (Plt_Vector3f){ 0.4f, 0.3f, 0.3f });
	plt_renderer_set_directional_lighting(renderer, (Plt_Vector3f){ 1.0f, 0.78f, 0.78f });
	plt_renderer_set_directional_lighting_direction(renderer, (Plt_Vector3f){-0.3f,-1.0f,0.1f});

	while (!plt_application_should_close(app)) {
		plt_application_update(app);
	}

	plt_mesh_destroy(&platypus_mesh);
	plt_texture_destroy(&platypus_texture);
	plt_application_destroy(&app);
	plt_world_destroy(&world);

	return 0;
}
