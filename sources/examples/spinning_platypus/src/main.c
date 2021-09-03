#include <stdio.h>

#include "platypus/platypus.h"
#include <math.h>

#define PLT_COMPONENT_SPINNING "spinning"
void _spinning_update(Plt_World *world, Plt_Entity_ID entity_id, void *instance_data, Plt_Frame_State state) {
	
	Plt_Transform transform = plt_world_entity_get_transform(world, entity_id);
	
	transform = plt_transform_rotate(transform, plt_quaternion_create_from_euler((Plt_Vector3f){0, 0.0005f * state.delta_time, 0}));
	transform.translation.y += sinf(state.application_time * 0.0025f) * 0.005f;
	
	plt_world_entity_set_transform(world, entity_id, transform);
}
void plt_register_spinning_component(Plt_World *world) {
	plt_world_register_component(world, PLT_COMPONENT_SPINNING, 0, NULL, _spinning_update, NULL);
}

#define PLT_COMPONENT_FPS_VIEWER "fps_viewer"
typedef struct Plt_Component_FPS_Viewer_Data {
	Plt_Font *font;
} Plt_Component_FPS_Viewer_Data;
void _fps_viewer_render(Plt_World *world, Plt_Entity_ID entity_id, void *instance_data, Plt_Frame_State state, Plt_Renderer *renderer) {
	Plt_Component_FPS_Viewer_Data *data = instance_data;
	char text[32];
	sprintf(text, "FPS:%d", (int)(1000 / state.delta_time));
	plt_renderer_direct_draw_text(renderer, (Plt_Vector2i){0, 0}, data->font, text);
}
void plt_register_fps_viewer_component(Plt_World *world) {
	plt_world_register_component(world, PLT_COMPONENT_FPS_VIEWER, sizeof(Plt_Component_FPS_Viewer_Data), NULL, NULL, _fps_viewer_render);
}
void plt_component_fps_viewer_set_font(Plt_World *world, Plt_Entity_ID entity_id, Plt_Font *font) {
	Plt_Component_FPS_Viewer_Data *data = plt_world_get_component_instance_data(world, entity_id, PLT_COMPONENT_FPS_VIEWER);
	if (!data) {
		return;
	}
	
	data->font = font;
}

int main(int argc, char **argv) {
	Plt_Application *app = plt_application_create("Platypus - Spinning Platypus", 860, 640, 1, Plt_Application_Option_None);
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
	Plt_World *world = plt_world_create();
	plt_register_spinning_component(world);
	plt_register_fps_viewer_component(world);
	plt_application_set_world(app, world);
	
	// Create world objects
	Plt_Entity_ID platypus_entity = plt_world_create_entity(world, "platypus", PLT_ENTITY_ID_NONE);
	plt_world_entity_set_transform(world, platypus_entity, plt_transform_create(plt_vector3f_make(0, 0, 0), plt_quaternion_create_from_euler(plt_vector3f_make(PLT_PI, 0, 0)), plt_vector3f_make(0.5f, 0.5f, 0.5f)));
	plt_world_entity_add_component(world, platypus_entity, PLT_COMPONENT_MESH_RENDERER);
	plt_component_mesh_renderer_set_mesh(world, platypus_entity, platypus_mesh);
	plt_component_mesh_renderer_set_texture(world, platypus_entity, platypus_texture);
	plt_world_entity_add_component(world, platypus_entity, PLT_COMPONENT_SPINNING);
	plt_world_entity_add_component(world, platypus_entity, PLT_COMPONENT_COLLIDER);
	plt_component_collider_set_box_shape(world, platypus_entity, plt_shape_box_make(plt_vector3f_make(1, 1, 1)));
	
	Plt_Entity_ID terrain_entity = plt_world_create_entity(world, "terrain", PLT_ENTITY_ID_NONE);
	plt_world_entity_set_transform(world, terrain_entity, plt_transform_create(plt_vector3f_make(0, 0, 0), plt_quaternion_create_from_euler(plt_vector3f_make(0, 0, 0)), plt_vector3f_make(1, 1, 1)));
	plt_world_entity_add_component(world, terrain_entity, PLT_COMPONENT_MESH_RENDERER);
	plt_component_mesh_renderer_set_mesh(world, terrain_entity, terrain_mesh);
	plt_component_mesh_renderer_set_texture(world, terrain_entity, lava_texture);
	
	Plt_Entity_ID camera_entity = plt_world_create_entity(world, "camera", PLT_ENTITY_ID_NONE);
	plt_world_entity_set_transform(world, camera_entity, plt_transform_create(plt_vector3f_make(0, -0.5f, 5.0f), plt_quaternion_create_from_euler(plt_vector3f_make(0, 0, 0)), plt_vector3f_make(1.0f, 1.0f, 1.0f)));
	plt_world_entity_add_component(world, camera_entity, "flying_camera_controller");
	plt_world_entity_add_component(world, camera_entity, PLT_COMPONENT_CAMERA);
	
	Plt_Entity_ID fps_viewer = plt_world_create_entity(world, "fps_viewer", PLT_ENTITY_ID_NONE);
	plt_world_entity_add_component(world, fps_viewer, PLT_COMPONENT_FPS_VIEWER);
	plt_component_fps_viewer_set_font(world, fps_viewer, font);

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
	plt_world_destroy(world);

	return 0;
}
