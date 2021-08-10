#pragma once

#include "platypus/platypus.h"
#include <stdlib.h>

#include "platypus/world/base_types/mesh_renderer/plt_object_type_mesh_renderer.h"
#include "platypus/world/base_types/camera/plt_object_type_camera.h"
#include "platypus/world/base_types/flying_camera_controller/plt_object_type_flying_camera_controller.h"

#define PLT_WORLD_BASE_TYPE_DESCRIPTOR_COUNT 3

void plt_world_get_base_type_descriptors(Plt_Object_Type_Descriptor *descriptors) {
	descriptors[0] = plt_object_type_mesh_renderer_get_descriptor();
	descriptors[1] = plt_object_type_camera_get_descriptor();
	descriptors[2] = plt_object_type_flying_camera_controller_get_descriptor();
}
