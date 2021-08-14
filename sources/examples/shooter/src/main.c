#include <stdio.h>

#include "platypus/platypus.h"
#include <math.h>

static Plt_Font *global_font;
static Plt_Texture *global_plasma_texture;

#define Plt_Object_Type_Spinning_Weapon 1
void _spinning_weapon_update(Plt_Object *object, void *type_data, Plt_Frame_State state) {
	if (plt_object_get_root(object) == object) {
		// Dropped weapon, do spinning animation:
		object->transform = plt_transform_rotate(object->transform, plt_quaternion_create_from_euler((Plt_Vector3f){0, 0.0005f * state.delta_time, 0}));
		object->transform.translation.y += sinf(state.application_time * 0.0025f) * 0.005f;
	} else {
		// Picked up, to idle animation:
		object->transform.translation.y += sinf(state.application_time * 0.0025f) * 0.001f;
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

#define Plt_Object_Type_Player_Controller 3
typedef struct Plt_Object_Type_Player_Controller_Data {
	Plt_Object *weapon_in_pick_up_range;
	Plt_Object *held_weapon;
	float weapon_cooldown;
} Plt_Object_Type_Player_Controller_Data;

void player_shoot_projectile(Plt_Object *player);
void _player_controller_update(Plt_Object *object, void *type_data, Plt_Frame_State state) {
	Plt_Object_Type_Player_Controller_Data *data = type_data;
	
	Plt_Vector3f global_position = plt_object_get_global_position(object);
	
	data->weapon_cooldown -= state.delta_time;
	data->weapon_in_pick_up_range = NULL;
	
	// Update weapon_in_pick_up_range
	{
		Plt_Object *weapons[32];
		unsigned int weapons_count;
		plt_world_get_objects_of_type(object->world, Plt_Object_Type_Spinning_Weapon, weapons, 32, &weapons_count);
		
		Plt_Object *closest_weapon = NULL;
		float closest_distance = INFINITY;
		for (unsigned int i = 0; i < weapons_count; ++i) {
			Plt_Object *weapon = weapons[i];
			if (weapon == data->held_weapon) {
				continue;
			}
			Plt_Vector3f weapon_position = plt_object_get_global_position(weapon);
			float distance = plt_vector3f_distance(global_position, weapon_position);
			if (distance < closest_distance) {
				closest_weapon = weapon;
				closest_distance = distance;
			}
		}
		
		if (closest_distance < 5.0f) {
			data->weapon_in_pick_up_range = closest_weapon;
		}
	}
	
	Plt_Key pressed = plt_input_state_get_pressed_keys(state.input_state);
	
	if (data->weapon_in_pick_up_range && (pressed & Plt_Key_E)) {
		if (data->held_weapon) {
			// Drop weapon
			plt_object_set_parent(data->held_weapon, NULL);
			
			Plt_Vector3f forward = plt_object_get_forward(object);
			data->held_weapon->transform.translation = plt_vector3f_add(global_position, plt_vector3f_multiply_scalar(forward, 3.0f));
		}
		
		data->held_weapon = data->weapon_in_pick_up_range;
		plt_object_set_parent(data->weapon_in_pick_up_range, object);
		data->weapon_in_pick_up_range->transform.translation = (Plt_Vector3f){ 0.5f, 0.4f, -2.0f };
		data->weapon_in_pick_up_range->transform.rotation = plt_quaternion_create_from_euler((Plt_Vector3f){ PLT_PI, plt_math_deg2rad(90.0f), 0 });
	}
	
	if (data->held_weapon) {
		if (plt_input_state_is_mouse_button_down(state.input_state, Plt_Mouse_Button_Left) && (data->weapon_cooldown <= 0)) {
			player_shoot_projectile(object);
			data->weapon_cooldown = 0.05f * 1000.0f;
		}
	}
}

void _player_controller_render_ui(Plt_Object *object, void *type_data, Plt_Frame_State state, Plt_Renderer *renderer) {
	Plt_Object_Type_Player_Controller_Data *data = type_data;
	
	if (data->weapon_in_pick_up_range) {
		char text[128];
		sprintf(text, "Press 'E' to pickup '%s'", data->weapon_in_pick_up_range->name);
		Plt_Size rendered_size = plt_font_get_size_of_string(global_font, text);
		Plt_Size viewport_size = plt_renderer_get_framebuffer_size(renderer);
		Plt_Vector2i pos = { viewport_size.width / 2 - rendered_size.width / 2, viewport_size.height - rendered_size.height - 64 };
		plt_renderer_direct_draw_text(renderer, pos, global_font, text);
	}
}

#define Plt_Object_Type_Projectile 4
typedef struct Plt_Object_Type_Projectile_Data {
	float lifetime;
} Plt_Object_Type_Projectile_Data;

#define Plt_Object_Type_Crosshair 5
typedef struct Plt_Object_Type_Crosshair_Data {
	Plt_Texture *crosshair_texture;
} Plt_Object_Type_Crosshair_Data;

#define Plt_Object_Type_Enemy 6
typedef struct Plt_Object_Type_Enemy_Data {
	bool idk;
} Plt_Object_Type_Enemy_Data;

void _enemy_update(Plt_Object *object, void *type_data, Plt_Frame_State state) {
	Plt_Object_Type_Enemy_Data *data = type_data;
	
	unsigned int collision_count = 0;
	Plt_Object **collisions = plt_object_get_collisions(object, &collision_count);
	for (unsigned int i = 0; i < collision_count; ++i) {
		Plt_Object *projectile = plt_object_get_root(collisions[i]);
		if (projectile->type == Plt_Object_Type_Projectile) {
			plt_object_destroy(&object);
			plt_object_destroy(&projectile);
		}
	}
}

void _crosshair_render_ui(Plt_Object *object, void *type_data, Plt_Frame_State state, Plt_Renderer *renderer) {
	Plt_Object_Type_Crosshair_Data *data = type_data;
	
	Plt_Size texture_size = plt_texture_get_size(data->crosshair_texture);
	Plt_Size viewport_size = plt_renderer_get_framebuffer_size(renderer);
	Plt_Rect rect = {
		.x = viewport_size.width / 2 - texture_size.width / 2,
		.y = viewport_size.height / 2 - texture_size.height / 2,
		.width = texture_size.width,
		.height = texture_size.height
	};
	
	plt_renderer_direct_draw_texture(renderer, rect, 0, data->crosshair_texture);
}

void _projectile_update(Plt_Object *object, void *type_data, Plt_Frame_State state) {
	Plt_Object_Type_Projectile_Data *data = type_data;
	
	data->lifetime -= state.delta_time;
	if (data->lifetime < 0) {
		plt_object_destroy(&object);
		return;
	}
	
	Plt_Vector3f forward = plt_object_get_forward(object);
	float projectile_speed = 0.1f;
	object->transform = plt_transform_translate(object->transform, plt_vector3f_multiply_scalar(forward, projectile_speed * state.delta_time));
}

void player_shoot_projectile(Plt_Object *player) {
	Plt_World *world = player->world;
	Plt_Object *projectile_origin = plt_object_get_object_at_path(player, "plasma_rifle/projectile_origin");
	
	Plt_Object *plasma_projectile = plt_object_create(world, NULL, Plt_Object_Type_Projectile, "plasma_projectile");
	plasma_projectile->transform.translation = plt_object_get_global_position(projectile_origin);
	plasma_projectile->transform.rotation = plt_object_get_root(projectile_origin)->transform.rotation;
	{
		Plt_Object_Type_Projectile_Data *data = plasma_projectile->type_data;
		data->lifetime = 2.0f * 1000.0f;
	}
	
	Plt_Object *plasma_projectile_renderer = plt_object_create(world, plasma_projectile, Plt_Object_Type_Billboard_Renderer, "mesh_renderer");
	{
		Plt_Object_Type_Billboard_Renderer_Data *data = plasma_projectile_renderer->type_data;
		data->size = plt_vector2f_make(0.5f, 0.5f);
		data->texture = global_plasma_texture;
	}
	
	Plt_Object *plasma_projectile_collider = plt_object_create(world, plasma_projectile, Plt_Object_Type_Collider, "collider");
	{
		Plt_Object_Type_Collider_Data *data = plasma_projectile_collider->type_data;
		data->shape_type = Plt_Shape_Type_Box;
		data->box_shape.size = plt_vector3f_make(0.5f, 0.5f, 0.5f);
	}
}

int main(int argc, char **argv) {
	Plt_Application *app = plt_application_create("Platypus - Shooter", 860, 640, 1, Plt_Application_Option_Fullscreen);
	plt_application_set_target_fps(app, 120);
	Plt_Renderer *renderer = plt_application_get_renderer(app);
	
	// Load assets
	Plt_Font *font = global_font = plt_font_load("assets/font_10x16.png");
	Plt_Mesh *platypus_mesh = plt_mesh_load_ply("assets/platypus.ply");
	Plt_Texture *platypus_texture = plt_texture_load("assets/platypus.png");
	Plt_Mesh *enemy_mesh = plt_mesh_load_ply("assets/wizard.ply");
	Plt_Texture *enemy_texture = plt_texture_load("assets/wizard.png");
	Plt_Mesh *gun_mesh = plt_mesh_load_ply("assets/gun.ply");
	Plt_Texture *gun_texture = plt_texture_load("assets/gun.png");
	Plt_Mesh *terrain_mesh = plt_mesh_load_ply("assets/terrain.ply");
	Plt_Texture *lava_texture = plt_texture_load("assets/lava.png");
	Plt_Texture *plasma_texture = global_plasma_texture = plt_texture_load("assets/plasma.png");
	Plt_Texture *crosshair_texture = plt_texture_load("assets/crosshair.png");

	// Create world
	const unsigned int type_descriptor_count = 6;
	Plt_Object_Type_Descriptor type_descriptors[6] = {
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
		},
		{ // Player Controller
			.id = Plt_Object_Type_Player_Controller,
			.data_size = sizeof(Plt_Object_Type_Player_Controller_Data),
			.update = _player_controller_update,
			.render_scene = NULL,
			.render_ui = _player_controller_render_ui
		},
		{ // Projectile
			.id = Plt_Object_Type_Projectile,
			.data_size = sizeof(Plt_Object_Type_Projectile_Data),
			.update = _projectile_update,
			.render_scene = NULL,
		},
		{ // Crosshair
			.id = Plt_Object_Type_Crosshair,
			.data_size = sizeof(Plt_Object_Type_Crosshair_Data),
			.update = NULL,
			.render_scene = NULL,
			.render_ui = _crosshair_render_ui
		},
		{ // Enemy
			.id = Plt_Object_Type_Enemy,
			.data_size = sizeof(Plt_Object_Type_Enemy_Data),
			.update = _enemy_update,
			.render_scene = NULL,
			.render_ui = NULL
		}
	};
	
	Plt_World *world = plt_world_create(2048, type_descriptors, type_descriptor_count);
	plt_application_set_world(app, world);
	
	// Create world objects
	Plt_Object *fps_viewer = plt_object_create(world, NULL, Plt_Object_Type_FPS_Viewer, "fps_viewer");
	{
		Plt_Object_Type_FPS_Viewer_Data *fps_viewer_data = fps_viewer->type_data;
		fps_viewer_data->font = font;
	}
	
	Plt_Object *crosshair = plt_object_create(world, NULL, Plt_Object_Type_Crosshair, "crosshair");
	{
		Plt_Object_Type_Crosshair_Data *data = crosshair->type_data;
		data->crosshair_texture = crosshair_texture;
	}
	
	Plt_Object *weapon_object = plt_object_create(world, NULL, Plt_Object_Type_Spinning_Weapon, "plasma_rifle");
	weapon_object->transform.translation = (Plt_Vector3f){0.45f, -0.5f, 1.0f};
	weapon_object->transform.rotation = plt_quaternion_create_from_euler((Plt_Vector3f){PLT_PI, 0, 0});
	weapon_object->transform.scale = (Plt_Vector3f){0.4f, 0.4f, 0.4f};
	Plt_Object *weapon_mesh_renderer = plt_object_create(world, weapon_object, Plt_Object_Type_Mesh_Renderer, "plasma_rifle_renderer");
	{
		Plt_Object_Type_Mesh_Renderer_Data *mesh_type_data = weapon_mesh_renderer->type_data;
		mesh_type_data->mesh = gun_mesh;
		mesh_type_data->texture = gun_texture;
	}
	Plt_Object *weapon_projectile_origin = plt_object_create(world, weapon_object, Plt_Object_Type_None, "projectile_origin");
	weapon_projectile_origin->transform.translation = plt_vector3f_make(4, 0, 0.0f);
	
	Plt_Object *terrain_object = plt_object_create(world, NULL, Plt_Object_Type_Mesh_Renderer, "terrain");
	{
		Plt_Object_Type_Mesh_Renderer_Data *mesh_type_data = terrain_object->type_data;
		mesh_type_data->mesh = terrain_mesh;
		mesh_type_data->texture = lava_texture;
	}
	
	Plt_Object *flying_camera_object = plt_object_create(world, NULL, Plt_Object_Type_Flying_Camera_Controller, "flying_camera");
	flying_camera_object->transform.translation = (Plt_Vector3f){ 0.0f, -0.5f, 5.0f };
	{
		Plt_Object_Type_Flying_Camera_Controller_Data *flying_camera_type_data = flying_camera_object->type_data;
		flying_camera_type_data->speed = 5.0f;
	}
	
	Plt_Object *player_object = plt_object_create(world, flying_camera_object, Plt_Object_Type_Player_Controller, "player");
	
	Plt_Object *camera_object = plt_object_create(world, flying_camera_object, Plt_Object_Type_Camera, "main_camera");
	Plt_Object_Type_Camera_Data *camera_type_data = camera_object->type_data;
	camera_type_data->fov = plt_math_deg2rad(150.0f);
	camera_type_data->near_z = 0.1f;
	camera_type_data->far_z = 100.0f;
	
	int count = 4;
	for (int x = -count; x < count; ++x) {
		for (int z = -count; z < count; ++z) {
			Plt_Object *enemy_object = plt_object_create(world, NULL, Plt_Object_Type_Enemy, "enemy");
			enemy_object->transform.translation = plt_vector3f_make(x, 0.0f, z - 10);
			enemy_object->transform.rotation = plt_quaternion_create_from_euler(plt_vector3f_make(PLT_PI, 0, 0));
			enemy_object->transform.scale = plt_vector3f_make(0.5f, 0.5f, 0.5f);
			Plt_Object *enemy_renderer = plt_object_create(world, enemy_object, Plt_Object_Type_Mesh_Renderer, "renderer");
			{
				Plt_Object_Type_Mesh_Renderer_Data *data = enemy_renderer->type_data;
				data->mesh = enemy_mesh;
				data->texture = enemy_texture;
			}
			Plt_Object *enemy_collider = plt_object_create(world, enemy_object, Plt_Object_Type_Collider, "collider");
			enemy_collider->transform.translation = plt_vector3f_make(0, 0.7f, 0);
			{
				Plt_Object_Type_Collider_Data *data = enemy_collider->type_data;
				data->shape_type = Plt_Shape_Type_Box;
				data->box_shape.size = plt_vector3f_make(1.8f, 6.5f, 2.0f);
			}
		}
	}

	// Setup lighting
	// This will be refactored into being part of the world eventually
	plt_renderer_set_ambient_lighting(renderer, (Plt_Vector3f){ 0.4f, 0.3f, 0.3f });
	plt_renderer_set_directional_lighting(renderer, (Plt_Vector3f){ 1.0f, 0.78f, 0.78f });
	plt_renderer_set_directional_lighting_direction(renderer, (Plt_Vector3f){-0.3f,-1.0f,0.1f});

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
