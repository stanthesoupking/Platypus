#pragma once

#include "platypus/platypus.h"

void _mesh_renderer_type_render(Plt_Object *object, void *type_data, Plt_Renderer *renderer);

#define PLT_OBJECT_TYPE_MESH_RENDERER_DESCRIPTOR \
{ \
	.id = Plt_Object_Type_Mesh_Renderer, \
	.data_size = sizeof(Plt_Object_Type_Mesh_Renderer_Data), \
	.update = NULL, \
	.render = _mesh_renderer_type_render \
}