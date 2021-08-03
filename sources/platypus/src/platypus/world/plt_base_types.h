#pragma once

#include "platypus/platypus.h"
#include <stdlib.h>

void _mesh_type_render(Plt_Object *object, Plt_Renderer *renderer);

const static unsigned int base_type_descriptor_count = 1;
const static Plt_Object_Type_Descriptor base_type_descriptors[] = {
	{ // Mesh
		.id = Plt_Object_Type_Mesh,
		.data_size = sizeof(Plt_Object_Type_Mesh_Data),
		.update = NULL,
		.render = _mesh_type_render
	}
};

void _mesh_type_render(Plt_Object *object, Plt_Renderer *renderer) {
	Plt_Object_Type_Mesh_Data *mesh_type_data = (Plt_Object_Type_Mesh_Data *)object->type_data;
	
	if (!mesh_type_data->mesh) {
		return;
	}
	
	plt_renderer_set_model_matrix(renderer, plt_transform_to_matrix(object->transform));
	plt_renderer_bind_texture(renderer, mesh_type_data->texture);
	plt_renderer_set_render_color(renderer, mesh_type_data->color);
	plt_renderer_set_primitive_type(renderer, Plt_Primitive_Type_Triangle);
	plt_renderer_draw_mesh(renderer, mesh_type_data->mesh);
}
