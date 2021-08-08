#include "plt_object_type_camera.h"

Plt_Matrix4x4f plt_object_type_camera_get_view_matrix(Plt_Object *camera) {
	// TODO: Assert that this is a camera object

	Plt_Transform inverse_transform = plt_transform_invert(camera->transform);
	inverse_transform.scale = (Plt_Vector3f){1, 1, 1};

	return plt_transform_to_matrix(inverse_transform);
}

Plt_Matrix4x4f plt_object_type_camera_get_projection_matrix(Plt_Object *camera, Plt_Vector2i viewport) {
	// TODO: Assert that this is a camera object
	Plt_Object_Type_Camera_Data *camera_type_data = (Plt_Object_Type_Camera_Data *)camera->type_data;

	float aspect_ratio = viewport.x / (float)viewport.y;
	return plt_matrix_perspective_make(aspect_ratio, camera_type_data->fov, camera_type_data->near_z, camera_type_data->far_z);
}
