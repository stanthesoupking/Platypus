#pragma once

#include "platypus/platypus.h"
#include <stdlib.h>

#include "platypus/world/base_types/mesh_renderer/plt_object_type_mesh_renderer.h"
#include "platypus/world/base_types/camera/plt_object_type_camera.h"

void _mesh_type_render(Plt_Object *object, void *data, Plt_Renderer *renderer);

const static unsigned int base_type_descriptor_count = 2;
const static Plt_Object_Type_Descriptor base_type_descriptors[] = {
	PLT_OBJECT_TYPE_MESH_RENDERER_DESCRIPTOR,
	PLT_OBJECT_TYPE_CAMERA_DESCRIPTOR
};
