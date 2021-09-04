#include "plt_component_camera.h"

#include <stdlib.h>
#include "platypus/base/plt_macros.h"
#include "platypus/world/plt_world.h"

void _camera_init(Plt_World *world, Plt_Entity_ID entity_id, void *instance_data) {
	Plt_Object_Type_Camera_Data *data = instance_data;
	data->fov = plt_math_deg2rad(70.0f);
	data->near_z = 0.1f;
	data->far_z = 100.0f;
}

void plt_register_camera_component(Plt_World *world) {
	plt_world_register_component(world, PLT_COMPONENT_CAMERA, sizeof(Plt_Object_Type_Camera_Data), _camera_init, NULL, NULL);
}

Plt_Matrix4x4f plt_component_camera_get_view_matrix(Plt_World *world, Plt_Entity_ID entity_id) {
	return plt_matrix_invert(plt_world_entity_get_model_matrix(world, entity_id));
}

Plt_Matrix4x4f plt_component_camera_get_projection_matrix(Plt_World *world, Plt_Entity_ID entity_id, Plt_Size viewport) {
	Plt_Object_Type_Camera_Data *data = plt_world_get_component_instance_data(world, entity_id, PLT_COMPONENT_CAMERA);
	if (!data) {
		return plt_matrix_identity();
	}
	
	float aspect_ratio = viewport.width / (float)viewport.height;
	return plt_matrix_perspective_make(aspect_ratio, data->fov, data->near_z, data->far_z);
}
