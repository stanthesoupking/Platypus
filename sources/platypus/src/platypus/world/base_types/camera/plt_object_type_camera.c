#include "plt_object_type_camera.h"

#include <stdlib.h>

Plt_Matrix4x4f plt_object_type_camera_get_view_matrix(Plt_Object *camera) {
	// TODO: Assert that this is a camera object
	Plt_Transform t = plt_object_get_parent(camera)->transform;
	t.rotation = plt_quaternion_invert(t.rotation);
	t.translation = plt_vector3f_multiply_scalar(t.translation, -1.0f);

	return plt_matrix_multiply(plt_quaternion_to_matrix(t.rotation), plt_matrix_translate_make(t.translation));
}

Plt_Matrix4x4f plt_object_type_camera_get_projection_matrix(Plt_Object *camera, Plt_Vector2i viewport) {
	// TODO: Assert that this is a camera object
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
