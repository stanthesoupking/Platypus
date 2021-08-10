#include "plt_object.h"

#include "plt_world.h"

Plt_Object *plt_object_get_parent(Plt_Object *object) {
	return plt_world_get_object_parent(object->world, object);
}

Plt_Matrix4x4f plt_object_get_model_matrix(Plt_Object *object) {
	Plt_Matrix4x4f parent_matrix = plt_world_get_object_parent_matrix(object->world, object);
	return plt_matrix_multiply(parent_matrix, plt_transform_to_matrix(object->transform));
}

Plt_Vector3f plt_object_get_forward(Plt_Object *object) {
	Plt_Vector4f v = { 0.0f, 0.0f, -1.0f, 0.0f };
	Plt_Matrix4x4f model = plt_object_get_model_matrix(object);
	Plt_Vector4f result = plt_matrix_multiply_vector4f(model, v);
	return (Plt_Vector3f){ result.x, result.y, result.z };
}

Plt_Vector3f plt_object_get_up(Plt_Object *object) {
	Plt_Vector4f v = { 0.0f, 1.0f, 0.0f, 0.0f };
	Plt_Matrix4x4f model = plt_object_get_model_matrix(object);
	Plt_Vector4f result = plt_matrix_multiply_vector4f(model, v);
	return (Plt_Vector3f){ result.x, result.y, result.z };
}

Plt_Vector3f plt_object_get_right(Plt_Object *object) {
	Plt_Vector4f v = { 1.0f, 0.0f, 0.0f, 0.0f };
	Plt_Matrix4x4f model = plt_object_get_model_matrix(object);
	Plt_Vector4f result = plt_matrix_multiply_vector4f(model, v);
	return (Plt_Vector3f){ result.x, result.y, result.z };
}