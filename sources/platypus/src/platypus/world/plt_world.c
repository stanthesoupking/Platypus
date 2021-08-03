#include "plt_world.h"

#include <stdlib.h>
#include "platypus/base/macros.h"

void plt_world_register_object_type(Plt_World *world, Plt_Object_Type_Descriptor descriptor);

Plt_World *plt_world_create(unsigned int object_storage_capacity, Plt_Object_Type_Descriptor *type_descriptors, unsigned int type_descriptor_count) {
	plt_assert(object_storage_capacity > 0, "Object storage capacity must be at least 1");

	Plt_World *world = malloc(sizeof(Plt_World));

	world->object_ref_list.root = NULL;

	world->object_count = 0;
	world->object_storage_capacity = object_storage_capacity;
	world->inclusive_object_type_data_size = 0;
	world->object_size = sizeof(Plt_Object);

	for (int i = 0; i < PLT_WORLD_MAX_OBJECT_TYPES; ++i) {
		world->registered_object_types[i].is_set = false;
		world->registered_object_types[i].descriptor.update = NULL;
		world->registered_object_types[i].descriptor.render = NULL;
	}
	for (int i = 0; i < type_descriptor_count; ++i) {
		plt_world_register_object_type(world, type_descriptors[i]);
	}

	world->object_storage = malloc(world->object_size * object_storage_capacity);
	world->object_storage_is_set = malloc(sizeof(bool) * object_storage_capacity);

	for (int i = 0; i < object_storage_capacity; ++i) {
		world->object_storage_is_set[i] = false;
	}

	return world;
}

void plt_world_destroy(Plt_World **world) {
	free((*world)->object_storage_is_set);
	free((*world)->object_storage);
	free(*world);
	*world = NULL;
}

unsigned int plt_world_get_object_index(Plt_World *world, Plt_Object *object) {
	return ((char *)object - world->object_storage) / world->object_size;
}

Plt_Object *plt_world_get_object_at_index(Plt_World *world, unsigned int index) {
	return (Plt_Object *)(world->object_storage + index * world->object_size);
}

Plt_Object *plt_world_create_object(Plt_World *world, Plt_Object_Type_ID type, const char *name) {
	plt_assert(world->object_count < world->object_storage_capacity, "World's object capacity has been exceeded.\n");

	unsigned int object_index;
	for (int i = 0; i < world->object_storage_capacity; ++i) {
		if (!world->object_storage_is_set[i]) {
			object_index = i;
			break;
		}
	}

	Plt_Object *object = plt_world_get_object_at_index(world, object_index);
	object->type = type;
	object->name = name;
	object->parent = NULL;
	object->transform = (Plt_Transform) {
		.translation = (Plt_Vector3f){0.0f, 0.0f, 0.0f},
		.rotation = (Plt_Vector3f){0.0f, 0.0f, 0.0f},
		.scale = (Plt_Vector3f){1.0f, 1.0f, 1.0f},
	};
	world->object_storage_is_set[object_index] = true;
	world->object_count++;

	return object;
}

void plt_world_destroy_object(Plt_World *world, Plt_Object **object) {
	unsigned int object_index = plt_world_get_object_index(world, *object);
	plt_assert(object_index < world->object_storage_capacity, "Object is outside of world's object capacity\n");
	
	if (world->object_storage_is_set[object_index]) {
		world->object_storage_is_set[object_index] = false;
		world->object_count--;
	}

	*object = NULL;
}

void plt_world_register_object_type(Plt_World *world, Plt_Object_Type_Descriptor descriptor) {
	plt_assert(!world->registered_object_types[descriptor.id].is_set, "Object ID is already in use.");

	world->inclusive_object_type_data_size = plt_max(world->inclusive_object_type_data_size, descriptor.data_size);
	world->object_size = sizeof(Plt_Object) + world->inclusive_object_type_data_size;
	
	world->registered_object_types[descriptor.id] = (Plt_Registered_Object_Type) {
		.is_set = true,
		.descriptor = descriptor
	};
}

void plt_world_update(Plt_World *world) {
	for(unsigned int i = 0; i < world->object_storage_capacity; ++i) {
		if (!world->object_storage_is_set[i]) {
			continue;
		}

		Plt_Object *object = plt_world_get_object_at_index(world, i);
		void (*update_func)(Plt_Object *object) = world->registered_object_types[object->type].descriptor.update;
		if (update_func) {
			update_func(object);
		}
	}
}

void plt_world_render(Plt_World *world, Plt_Renderer *renderer) {
	for(unsigned int i = 0; i < world->object_storage_capacity; ++i) {
		if (!world->object_storage_is_set[i]) {
			continue;
		}

		Plt_Object *object = plt_world_get_object_at_index(world, i);
		void (*render_func)(Plt_Object *object, Plt_Renderer *renderer) = world->registered_object_types[object->type].descriptor.render;
		if (render_func) {
			render_func(object, renderer);
		}
	}
}
