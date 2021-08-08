#pragma once

#include "platypus/platypus.h"

Plt_Object_Type_Descriptor plt_object_type_camera_get_descriptor();

Plt_Matrix4x4f plt_object_type_camera_get_view_matrix(Plt_Object *camera);
Plt_Matrix4x4f plt_object_type_camera_get_projection_matrix(Plt_Object *camera, Plt_Vector2i viewport);
