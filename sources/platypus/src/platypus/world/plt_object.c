#include "plt_object.h"

#include "plt_world.h"

Plt_Object *plt_object_create(Plt_World *world, Plt_Object *parent, Plt_Object_Type_ID type, const char *name) {
	plt_assert(world, "An object can't be created without a world.");
	if (parent) {
		plt_assert(parent->world == world, "Object parents must be in the same world as the child object.\n");
	}
	return plt_world_create_object(world, parent, type, name);
}

void plt_object_destroy(Plt_Object **object) {
	plt_world_destroy_object((*object)->world, object, true);
}

Plt_Object *plt_object_get_parent(Plt_Object *object) {
	return plt_world_get_object_parent(object->world, object);
}

Plt_Matrix4x4f plt_object_get_model_matrix(Plt_Object *object) {
	Plt_Matrix4x4f parent_matrix = plt_world_get_object_parent_matrix(object->world, object);
	return plt_matrix_multiply(parent_matrix, plt_transform_to_matrix(object->transform));
}

Plt_Object **plt_object_get_collisions(Plt_Object *object, unsigned int *collision_count) {
	return plt_world_get_object_collisions(object->world, object, collision_count);
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
