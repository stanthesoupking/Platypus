#include "plt_object_type_flying_camera_controller.h"

#include <stdlib.h>

void _flying_camera_controller_type_update(Plt_Object *object, void *type_data, Plt_Frame_State state) {
	Plt_Object_Type_Flying_Camera_Controller_Data *data = (Plt_Object_Type_Flying_Camera_Controller_Data *)type_data;

	Plt_Vector3f forwards = plt_object_get_forward(object);
	Plt_Vector3f right = plt_object_get_right(object);

	float adjusted_speed = data->speed * 0.01f * state.delta_time;

	Plt_Key pressed = plt_input_state_get_pressed_keys(state.input_state);
	if (pressed & Plt_Key_W) {
		object->transform.translation = plt_vector3f_add(object->transform.translation, plt_vector3f_multiply_scalar(forwards, adjusted_speed));
	}
	
	if (pressed & Plt_Key_S) {
		object->transform.translation = plt_vector3f_add(object->transform.translation, plt_vector3f_multiply_scalar(forwards, -adjusted_speed));
	}
	
	if (pressed & Plt_Key_A) {
		object->transform.translation = plt_vector3f_add(object->transform.translation, plt_vector3f_multiply_scalar(right, -adjusted_speed * 0.5f));
	}
	
	if (pressed & Plt_Key_D) {
		object->transform.translation = plt_vector3f_add(object->transform.translation, plt_vector3f_multiply_scalar(right, adjusted_speed * 0.5f));
	}
	
	Plt_Vector2f mouse_movement = plt_input_state_get_mouse_movement(state.input_state);
	data->yaw -= mouse_movement.x * 0.001f;
	data->pitch += mouse_movement.y * 0.001f;
	
	// Prevent camera from becoming flipped
	data->pitch = plt_clamp(data->pitch, -0.5f * PLT_PI, 0.5f * PLT_PI);

	object->transform.rotation = plt_quaternion_create_from_euler((Plt_Vector3f){data->pitch, data->yaw, 0});
}

Plt_Object_Type_Descriptor plt_object_type_flying_camera_controller_get_descriptor() {
	return (Plt_Object_Type_Descriptor) {
		.id = Plt_Object_Type_Flying_Camera_Controller,
		.data_size = sizeof(Plt_Object_Type_Flying_Camera_Controller_Data),
		.update = _flying_camera_controller_type_update,
		.render_scene = NULL,
		.render_ui = NULL
	};
}
