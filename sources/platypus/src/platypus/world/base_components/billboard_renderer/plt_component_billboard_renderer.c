#include "plt_component_billboard_renderer.h"

#include <stdlib.h>

void _billboard_renderer_type_render(Plt_World *world, Plt_Entity_ID entity_id, void *instance_data, Plt_Frame_State state, Plt_Renderer *renderer) {
	Plt_Object_Type_Billboard_Renderer_Data *data = instance_data;
	
//	plt_renderer_set_model_matrix(renderer, plt_object_get_model_matrix(object));
	plt_renderer_bind_texture(renderer, data->texture);
	plt_renderer_draw_billboard(renderer, data->size);
}

void plt_register_billboard_renderer_component(Plt_World *world) {
	plt_world_register_component(world, "billboard_renderer", sizeof(Plt_Object_Type_Billboard_Renderer_Data), NULL, NULL, _billboard_renderer_type_render);
}
