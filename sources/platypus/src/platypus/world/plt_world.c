#include "plt_world.h"

#include <stdlib.h>
#include "platypus/base/macros.h"
#include "plt_base_types.h"

void plt_world_register_object_type(Plt_World *world, Plt_Object_Type_Descriptor descriptor);

Plt_World *plt_world_create(unsigned int object_storage_capacity, Plt_Object_Type_Descriptor *type_descriptors, unsigned int type_descriptor_count, bool include_base_types) {
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

	if (include_base_types) {
		for (int i = 0; i < base_type_descriptor_count; ++i) {
			plt_world_register_object_type(world, base_type_descriptors[i]);
		}
	}

	world->object_storage = malloc(world->object_size * object_storage_capacity);
	world->object_storage_private_data = malloc(sizeof(Plt_Object_Private_Data) * object_storage_capacity);

	for (int i = 0; i < object_storage_capacity; ++i) {
		world->object_storage_private_data[i].is_set = false;
	}

	return world;
}

void plt_world_destroy(Plt_World **world) {
	free((*world)->object_storage_private_data);
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
		if (!world->object_storage_private_data[i].is_set) {
			object_index = i;
			break;
		}
	}

	Plt_Object *object = plt_world_get_object_at_index(world, object_index);
	object->world = world;
	object->type = type;
	object->name = name;
	object->parent = NULL;
	object->transform = (Plt_Transform) {
		.translation = (Plt_Vector3f){0.0f, 0.0f, 0.0f},
		.rotation = (Plt_Vector3f){0.0f, 0.0f, 0.0f},
		.scale = (Plt_Vector3f){1.0f, 1.0f, 1.0f},
	};
	world->object_storage_private_data[object_index].is_set = true;
	world->object_count++;

	return object;
}

void plt_world_destroy_object(Plt_World *world, Plt_Object **object) {
	unsigned int object_index = plt_world_get_object_index(world, *object);
	plt_assert(object_index < world->object_storage_capacity, "Object is outside of world's object capacity\n");
	
	if (world->object_storage_private_data[object_index].is_set) {
		world->object_storage_private_data[object_index].is_set = false;
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

Plt_Matrix4x4f plt_world_update_object_matrix_recursive(Plt_World *world, Plt_Object *object) {
	if (object == NULL) {
		return plt_matrix_identity();
	}

	unsigned int object_index = plt_world_get_object_index(world, object);
	Plt_Object_Private_Data *private_data = &world->object_storage_private_data[object_index];
	if (private_data->valid_matrices) {
		return plt_matrix_multiply(private_data->parent_matrix, plt_transform_to_matrix(object->transform));
	} else {
		private_data->parent_matrix = plt_world_update_object_matrix_recursive(world, object->parent);
		private_data->valid_matrices = true;
		return plt_matrix_multiply(private_data->parent_matrix, plt_transform_to_matrix(object->transform));
	}
}

void plt_world_update_object_matrices(Plt_World *world) {
	// Invalidate all matrices
	for (unsigned int i = 0; i < world->object_storage_capacity; ++i) {
		world->object_storage_private_data[i].valid_matrices = false;
	}

	// Update matrices
	for (unsigned int i = 0; i < world->object_storage_capacity; ++i) {
		if (!world->object_storage_private_data[i].is_set) {
			continue;
		}

		if (!world->object_storage_private_data[i].valid_matrices) {
			Plt_Object *object = plt_world_get_object_at_index(world, i);
			plt_world_update_object_matrix_recursive(world, object);
		}
	}
}

void plt_world_update(Plt_World *world) {
	plt_world_update_object_matrices(world);

	for (unsigned int i = 0; i < world->object_storage_capacity; ++i) {
		if (!world->object_storage_private_data[i].is_set) {
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
	plt_world_update_object_matrices(world);

	for (unsigned int i = 0; i < world->object_storage_capacity; ++i) {
		if (!world->object_storage_private_data[i].is_set) {
			continue;
		}

		Plt_Object *object = plt_world_get_object_at_index(world, i);
		void (*render_func)(Plt_Object *object, Plt_Renderer *renderer) = world->registered_object_types[object->type].descriptor.render;
		if (render_func) {
			render_func(object, renderer);
		}
	}
}

Plt_Matrix4x4f plt_world_get_object_parent_matrix(Plt_World *world, Plt_Object *object) {
	unsigned int object_index = plt_world_get_object_index(world, object);
	return world->object_storage_private_data[object_index].parent_matrix;
}