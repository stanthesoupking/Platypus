#include "plt_object.h"

#include "plt_world.h"

Plt_Object *plt_object_create(Plt_World *world, Plt_Object_Type_ID type, const char *name) {
	Plt_Object *object = malloc(sizeof(Plt_Object));
	object->ref = plt_world_create_object(world, type, name);
	return object;
}

void plt_object_destroy(Plt_Object **object) {
	plt_world_destroy_object((*object)->ref->world, &((*object)->ref));
	free(*object);
	*object = NULL;
}

void plt_object_set_name(Plt_Object *object, const char *name) {
	plt_world_dereference_object(object->ref)->name = name;
}

const char *plt_object_get_name(Plt_Object *object) {
	return plt_world_dereference_object(object->ref)->name;
}

void plt_object_set_type(Plt_Object *object, Plt_Object_Type_ID type) {
	plt_world_dereference_object(object->ref)->type = type;
}

Plt_Object_Type_ID plt_object_get_type(Plt_Object *object) {
	return plt_world_dereference_object(object->ref)->type;
}

void plt_object_set_translation(Plt_Object *object, Plt_Vector3f translation) {
	plt_world_dereference_object(object->ref)->transform.translation = translation;
}

Plt_Vector3f plt_object_get_translation(Plt_Object *object) {
	return plt_world_dereference_object(object->ref)->transform.translation;
}

void plt_object_set_rotation(Plt_Object *object, Plt_Vector3f rotation) {
	plt_world_dereference_object(object->ref)->transform.rotation = rotation;
}

Plt_Vector3f plt_object_get_rotation(Plt_Object *object) {
	return plt_world_dereference_object(object->ref)->transform.rotation;
}

void plt_object_set_scale(Plt_Object *object, Plt_Vector3f scale) {
	plt_world_dereference_object(object->ref)->transform.scale = scale;
}

Plt_Vector3f plt_object_get_scale(Plt_Object *object) {
	return plt_world_dereference_object(object->ref)->transform.scale;
}