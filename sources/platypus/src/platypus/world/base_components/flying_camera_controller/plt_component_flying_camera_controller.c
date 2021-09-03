#include "plt_component_flying_camera_controller.h"

#include <stdlib.h>

void _flying_camera_controller_type_init(Plt_World *world, Plt_Entity_ID entity_id, void *instance_data) {
	Plt_Object_Type_Flying_Camera_Controller_Data *data = instance_data;
	data->speed = 1.0f;
}

void _flying_camera_controller_type_update(Plt_World *world, Plt_Entity_ID entity_id, void *instance_data, Plt_Frame_State state) {
	Plt_Object_Type_Flying_Camera_Controller_Data *data = instance_data;

	Plt_Vector3f forwards = plt_entity_get_forward(world, entity_id);
	Plt_Vector3f right = plt_entity_get_right(world, entity_id);

	float adjusted_speed = data->speed * 0.01f * state.delta_time;
	
	Plt_Transform transform = plt_world_entity_get_transform(world, entity_id);

	Plt_Key pressed = plt_input_state_get_pressed_keys(state.input_state);
	if (pressed & Plt_Key_W) {
		transform.translation = plt_vector3f_add(transform.translation, plt_vector3f_multiply_scalar(forwards, adjusted_speed));
	}

	if (pressed & Plt_Key_S) {
		transform.translation = plt_vector3f_add(transform.translation, plt_vector3f_multiply_scalar(forwards, -adjusted_speed));
	}

	if (pressed & Plt_Key_A) {
		transform.translation = plt_vector3f_add(transform.translation, plt_vector3f_multiply_scalar(right, -adjusted_speed));
	}

	if (pressed & Plt_Key_D) {
		transform.translation = plt_vector3f_add(transform.translation, plt_vector3f_multiply_scalar(right, adjusted_speed));
	}

	Plt_Vector2f mouse_movement = plt_input_state_get_mouse_movement(state.input_state);
	data->yaw -= mouse_movement.x * 0.001f;
	data->pitch += mouse_movement.y * 0.001f;

	// Prevent camera from becoming flipped
	data->pitch = plt_clamp(data->pitch, -0.5f * PLT_PI, 0.5f * PLT_PI);

	transform.rotation = plt_quaternion_create_from_euler((Plt_Vector3f){data->pitch, data->yaw, 0});
	
	plt_world_entity_set_transform(world, entity_id, transform);
}

void plt_register_flying_camera_component(Plt_World *world) {
	plt_world_register_component(world, "flying_camera_controller", sizeof(Plt_Object_Type_Flying_Camera_Controller_Data), _flying_camera_controller_type_init, _flying_camera_controller_type_update, NULL);
}
