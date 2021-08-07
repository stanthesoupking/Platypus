#include "plt_object.h"

#include "plt_world.h"

Plt_Object *plt_object_get_parent(Plt_Object *object) {
	return plt_world_get_object_parent(object->world, object);
}

Plt_Matrix4x4f plt_object_get_model_matrix(Plt_Object *object) {
	Plt_Matrix4x4f parent_matrix = plt_world_get_object_parent_matrix(object->world, object);
	return plt_matrix_multiply(parent_matrix, plt_transform_to_matrix(object->transform));
}
