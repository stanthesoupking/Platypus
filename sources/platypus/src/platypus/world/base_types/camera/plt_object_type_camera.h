#pragma once

#include "platypus/platypus.h"

#define PLT_OBJECT_TYPE_CAMERA_DESCRIPTOR \
{ \
	.id = Plt_Object_Type_Camera, \
	.data_size = sizeof(Plt_Object_Type_Camera_Data), \
	.update = NULL, \
	.render = NULL \
}

Plt_Matrix4x4f plt_object_type_camera_get_view_matrix(Plt_Object *camera);
Plt_Matrix4x4f plt_object_type_camera_get_projection_matrix(Plt_Object *camera, Plt_Vector2i viewport);