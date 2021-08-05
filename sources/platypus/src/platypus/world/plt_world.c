#include "plt_world.h"

#include <stdlib.h>
#include <string.h>
#include "platypus/base/macros.h"
#include "plt_base_types.h"

void plt_world_register_object_type(Plt_World *world, Plt_Object_Type_Descriptor descriptor);

Plt_World *plt_world_create(unsigned int object_storage_capacity, Plt_Object_Type_Descriptor *type_descriptors, unsigned int type_descriptor_count, bool include_base_types) {
	plt_assert(object_storage_capacity > 0, "Object storage capacity must be >= 1");

	Plt_World *world = malloc(sizeof(Plt_World));

	world->object_list.root = NULL;
	world->object_count = 0;
	world->object_storage_capacity = object_storage_capacity;

	world->type_count = 0;
	int total_types = (include_base_types ? base_type_descriptor_count : 0) + type_descriptor_count;
	world->types = malloc(sizeof(Plt_Registered_Object_Type) * total_types);

	// Register object types
	if (include_base_types) {
		for (int i = 0; i < base_type_descriptor_count; ++i) {
			plt_world_register_object_type(world, base_type_descriptors[i]);
		}
	}
	for (int i = 0; i < type_descriptor_count; ++i) {
		plt_world_register_object_type(world, type_descriptors[i]);
	}

	// Check for duplicated object IDs
	for (int i = 0; i < world->type_count; ++i) {
		Plt_Object_Type_ID id = world->types[i].id;
		for (int j = 0; j < world->type_count; ++j) {
			if (j == i) {
				continue;
			}

			if (world->types[j].id == id) {
				printf("Duplicate object types with ID %d\n", id);
				abort();
			}
		}
	}

	world->object_storage = malloc(sizeof(Plt_Object) * object_storage_capacity);
	world->object_storage_private_data = malloc(sizeof(Plt_Object_Private_Data) * object_storage_capacity);

	for (int i = 0; i < object_storage_capacity; ++i) {
		world->object_storage_private_data[i].is_set = false;
	}

	return world;
}

void plt_world_destroy(Plt_World **world) {
	for (int i = 0; i < (*world)->type_count; ++i) {
		void *object_data = (*world)->types[i].object_entries;
		if (object_data) {
			free(object_data);
		}
	}
	free((*world)->object_storage_private_data);
	free((*world)->object_storage);
	free(*world);
	*world = NULL;
}

unsigned int plt_world_get_object_index(Plt_World *world, Plt_Object *object) {
	return (object - world->object_storage);
}

Plt_Object *plt_world_get_object_at_index(Plt_World *world, unsigned int index) {
	return (Plt_Object *)(world->object_storage + index);
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
	object->type_data = NULL;
	world->object_storage_private_data[object_index].is_set = true;
	world->object_count++;
	
	Plt_Linked_List_Node *node = malloc(sizeof(Plt_Linked_List_Node));
	node->data = object;
	node->next = world->object_list.root;
	world->object_list.root = node;
	
	// Insert into type data
	if (type != Plt_Object_Type_None) {
		Plt_Registered_Object_Type *registered_type = NULL;
		for (unsigned int i = 0; i < world->type_count; ++i) {
			if (world->types[i].id == type) {
				registered_type = &world->types[i];
				break;
			}
		}
		if (!registered_type) {
			printf("Object created with unregistered/unknown object type: %d\n", type);
			abort();
		}
		Plt_Registered_Object_Type_Entry *entry = (Plt_Registered_Object_Type_Entry *)(registered_type->object_entries + registered_type->object_entry_count * registered_type->object_entry_stride);
		entry->object = object;
		memset(entry->data, 0, registered_type->descriptor.data_size);
		object->type_data = entry->data;
		registered_type->object_entry_count++;
	}

	return object;
}

void plt_world_destroy_object(Plt_World *world, Plt_Object **object) {
	unsigned int object_index = plt_world_get_object_index(world, *object);
	plt_assert(object_index < world->object_storage_capacity, "Object is outside of world's object capacity\n");
	
	if (world->object_storage_private_data[object_index].is_set) {
		world->object_storage_private_data[object_index].is_set = false;
		world->object_count--;
	}
	
	// TODO: Remove from object list
	Plt_Linked_List_Node *node = world->object_list.root;
	if (node->data == *object) {
		world->object_list.root = node->next;
		free(node);
	} else {
		while (node->next != NULL) {
			if (node->next->data == *object) {
				Plt_Linked_List_Node *removed = node->next;
				node->next = node->next->next;
				free(removed);
				break;
			}
			node = node->next;
		}
	}
	
	// Remove type data
	if ((*object)->type != Plt_Object_Type_None) {
		Plt_Registered_Object_Type *type = NULL;
		for (unsigned int i = 0; i < world->type_count; ++i) {
			if (world->types[i].id == (*object)->type) {
				type = &world->types[i];
				break;
			}
		}
		plt_assert(type, "Failed to find object type data storage.");
		
		int found_index = -1;
		Plt_Registered_Object_Type_Entry *entry = (Plt_Registered_Object_Type_Entry *)type->object_entries;
		for (unsigned int i = 0; i < type->object_entry_count; ++i) {
			if (entry->object == *object) {
				// Remove
				found_index = i;
				break;
			}
			entry = (Plt_Registered_Object_Type_Entry *)(((char *)entry) + type->object_entry_stride);
		}
		plt_assert(found_index != -1, "Failed to find object in type data storage.");
		
		// Shift down existing entries to replace deleted
		char *entry_data = type->object_entries;
		for (unsigned int i = found_index; i < type->object_entry_count - 1; ++i) {
			for (unsigned int j = 0; j < type->object_entry_stride; ++j) {
				entry_data[i * type->object_entry_stride + j] = entry_data[(i + 1) * type->object_entry_stride + j];
			}
		}
		
		type->object_entry_count--;
	}

	*object = NULL;
}

void plt_world_register_object_type(Plt_World *world, Plt_Object_Type_Descriptor descriptor) {
	unsigned int entry_stride = sizeof(Plt_Registered_Object_Type_Entry) + descriptor.data_size;

	world->types[world->type_count++] = (Plt_Registered_Object_Type) {
		.id = descriptor.id,
		.descriptor = descriptor,
		.object_entry_stride = entry_stride,
		.object_entries = malloc(entry_stride * world->object_storage_capacity)
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

	for (unsigned int i = 0; i < world->type_count; ++i) {
		Plt_Registered_Object_Type type = world->types[i];
		void (*update_func)(Plt_Object *object, void *type_data) = type.descriptor.update;
		if (!update_func) {
			continue;
		}
		
		Plt_Registered_Object_Type_Entry *entry = (Plt_Registered_Object_Type_Entry *)type.object_entries;
		for (unsigned int j = 0; type.object_entry_count; ++j) {
			update_func(entry->object, entry->data);
			
			entry = (Plt_Registered_Object_Type_Entry *)(((char *)entry) + type.object_entry_stride);
		}
	}
}

void plt_world_render(Plt_World *world, Plt_Renderer *renderer) {
	plt_world_update_object_matrices(world);

	for (unsigned int i = 0; i < world->type_count; ++i) {
		Plt_Registered_Object_Type type = world->types[i];
		void (*render_func)(Plt_Object *object, void *type_data, Plt_Renderer *renderer) = type.descriptor.render;
		if (!render_func) {
			continue;
		}
		
		Plt_Registered_Object_Type_Entry *entry = (Plt_Registered_Object_Type_Entry *)type.object_entries;
		for (unsigned int j = 0; j < type.object_entry_count; ++j) {
			render_func(entry->object, entry->data, renderer);
			
			entry = (Plt_Registered_Object_Type_Entry *)(((char *)entry) + type.object_entry_stride);
		}
	}
}

Plt_Matrix4x4f plt_world_get_object_parent_matrix(Plt_World *world, Plt_Object *object) {
	unsigned int object_index = plt_world_get_object_index(world, object);
	return world->object_storage_private_data[object_index].parent_matrix;
}
