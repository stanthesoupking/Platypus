#include "plt_object_type_billboard_renderer.h"

#include <stdlib.h>

void _billboard_renderer_type_render(Plt_Object *object, void *type_data, Plt_Frame_State state, Plt_Renderer *renderer) {
	Plt_Object_Type_Billboard_Renderer_Data *data = type_data;
	
	plt_renderer_set_model_matrix(renderer, plt_object_get_model_matrix(object));
	plt_renderer_bind_texture(renderer, data->texture);
	plt_renderer_draw_billboard(renderer, data->size);
}

Plt_Object_Type_Descriptor plt_object_type_billboard_renderer_get_descriptor() {
	return (Plt_Object_Type_Descriptor) {
		.id = Plt_Object_Type_Billboard_Renderer,
		.data_size = sizeof(Plt_Object_Type_Billboard_Renderer_Data),
		.update = NULL,
		.render_scene = _billboard_renderer_type_render,
		.render_ui = NULL
	};
}
