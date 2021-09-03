#include "plt_component_mesh_renderer.h"

#include <stdlib.h>
#include "platypus/world/plt_world.h"

void _mesh_renderer_type_render(Plt_World *world, Plt_Entity_ID entity_id, void *instance_data, Plt_Frame_State state, Plt_Renderer *renderer) {
	Plt_Object_Type_Mesh_Renderer_Data *mesh_type_data = instance_data;
	
	if (!mesh_type_data->mesh) {
		return;
	}
	
	plt_renderer_set_lighting_model(renderer, Plt_Lighting_Model_Vertex_Lit);
	plt_renderer_set_model_matrix(renderer, plt_world_entity_get_model_matrix(world, entity_id));
	plt_renderer_bind_texture(renderer, mesh_type_data->texture);
	plt_renderer_set_render_color(renderer, mesh_type_data->color);
	plt_renderer_set_primitive_type(renderer, Plt_Primitive_Type_Triangle);
	plt_renderer_draw_mesh(renderer, mesh_type_data->mesh);
}

void plt_register_mesh_renderer_component(Plt_World *world) {
	plt_world_register_component(world, PLT_COMPONENT_MESH_RENDERER, sizeof(Plt_Object_Type_Mesh_Renderer_Data), NULL, NULL, _mesh_renderer_type_render);
}

void plt_component_mesh_renderer_set_mesh(Plt_World *world, Plt_Entity_ID entity_id, Plt_Mesh *mesh) {
	Plt_Object_Type_Mesh_Renderer_Data *data = plt_world_get_component_instance_data(world, entity_id, PLT_COMPONENT_MESH_RENDERER);
	if (!data) {
		return;
	}
	
	data->mesh = mesh;
}

void plt_component_mesh_renderer_set_texture(Plt_World *world, Plt_Entity_ID entity_id, Plt_Texture *texture) {
	Plt_Object_Type_Mesh_Renderer_Data *data = plt_world_get_component_instance_data(world, entity_id, PLT_COMPONENT_MESH_RENDERER);
	if (!data) {
		return;
	}
	
	data->texture = texture;
}
