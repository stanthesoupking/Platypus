#include "plt_object_type_camera.h"

#include <stdlib.h>

Plt_Matrix4x4f plt_object_type_camera_get_view_matrix(Plt_Object *camera) {
	plt_assert(camera->type == Plt_Object_Type_Camera, "Object must be of type 'Plt_Object_Type_Camera'.\n");
	Plt_Matrix4x4f model = plt_object_get_model_matrix(camera);
	return plt_matrix_invert(model);
}

Plt_Matrix4x4f plt_object_type_camera_get_projection_matrix(Plt_Object *camera, Plt_Vector2i viewport) {
	plt_assert(camera->type == Plt_Object_Type_Camera, "Object must be of type 'Plt_Object_Type_Camera'.\n");
	Plt_Object_Type_Camera_Data *camera_type_data = (Plt_Object_Type_Camera_Data *)camera->type_data;

	float aspect_ratio = viewport.x / (float)viewport.y;
	return plt_matrix_perspective_make(aspect_ratio, camera_type_data->fov, camera_type_data->near_z, camera_type_data->far_z);
}

Plt_Object_Type_Descriptor plt_object_type_camera_get_descriptor() {
	return (Plt_Object_Type_Descriptor) {
		.id = Plt_Object_Type_Camera,
		.data_size = sizeof(Plt_Object_Type_Camera_Data),
		.update = NULL,
		.render_scene = NULL,
		.render_ui = NULL
	};
}
