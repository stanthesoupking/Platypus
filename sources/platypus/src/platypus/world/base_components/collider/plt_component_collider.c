#include "plt_component_collider.h"

#include "platypus/base/plt_defines.h"
#include <stdlib.h>

#if PLT_DEBUG_COLLIDERS
static Plt_Mesh *box_mesh = NULL;

void _collider_type_render(Plt_World *world, Plt_Entity_ID entity_id, void *instance_data, Plt_Frame_State state, Plt_Renderer *renderer) {
	Plt_Object_Type_Collider_Data *data = instance_data;
	
	unsigned int collision_count = 0;
//	plt_object_get_collisions(object, &collision_count);
	Plt_Color8 render_color = collision_count > 0 ? plt_color8_make(255, 255, 0, 255) : plt_color8_make(0, 255, 0, 255);

	if (data->shape_type == Plt_Shape_Type_Box) {
		if (box_mesh == NULL) {
			box_mesh = plt_mesh_create_cube(data->box_shape.size);
		}
		plt_renderer_bind_texture(renderer, NULL);
		plt_renderer_set_lighting_model(renderer, Plt_Lighting_Model_Unlit);
		plt_renderer_set_render_color(renderer, render_color);
		plt_renderer_set_model_matrix(renderer, plt_world_entity_get_model_matrix(world, entity_id));
		plt_renderer_set_primitive_type(renderer, Plt_Primitive_Type_Line);
		plt_renderer_draw_mesh(renderer, box_mesh);
	}
}
#endif

void plt_register_collider_component(Plt_World *world) {
	void (*render_scene)(Plt_World *world, Plt_Entity_ID entity_id, void *instance_data, Plt_Frame_State state, Plt_Renderer *renderer) = NULL;
#if PLT_DEBUG_COLLIDERS
	render_scene = _collider_type_render;
#endif
	
	plt_world_register_component(world, PLT_COMPONENT_COLLIDER, sizeof(Plt_Object_Type_Collider_Data), NULL, NULL, render_scene);
}

void plt_component_collider_set_box_shape(Plt_World *world, Plt_Entity_ID entity_id, Plt_Shape_Box box) {
	Plt_Object_Type_Collider_Data *data = plt_world_get_component_instance_data(world, entity_id, PLT_COMPONENT_COLLIDER);
	if (!data)  {
		return;
	}
	
	data->shape_type = Plt_Shape_Type_Box;
	data->box_shape = box;
}

void plt_component_collider_set_sphere_shape(Plt_World *world, Plt_Entity_ID entity_id, Plt_Shape_Sphere sphere) {
	Plt_Object_Type_Collider_Data *data = plt_world_get_component_instance_data(world, entity_id, PLT_COMPONENT_COLLIDER);
	if (!data)  {
		return;
	}
	
	data->shape_type = Plt_Shape_Type_Sphere;
	data->sphere_shape = sphere;
}
