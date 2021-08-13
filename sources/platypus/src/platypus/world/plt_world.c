#include "plt_world.h"

#include <stdlib.h>
#include <string.h>
#include "platypus/base/plt_macros.h"
#include "plt_base_types.h"
#include "platypus/renderer/plt_renderer.h"
#include "platypus/math/plt_collision.h"

void plt_world_register_object_type(Plt_World *world, Plt_Object_Type_Descriptor descriptor);

Plt_World *plt_world_create(unsigned int object_storage_capacity, Plt_Object_Type_Descriptor *type_descriptors, unsigned int type_descriptor_count) {
	plt_assert(object_storage_capacity > 0, "Object storage capacity must be >= 1");

	Plt_World *world = malloc(sizeof(Plt_World));

	world->object_list.root = NULL;
	world->object_count = 0;
	world->object_storage_capacity = object_storage_capacity;

	Plt_Object_Type_Descriptor base_type_descriptors[PLT_WORLD_BASE_TYPE_DESCRIPTOR_COUNT];
	plt_world_get_base_type_descriptors(base_type_descriptors); 

	world->type_count = 0;
	int total_types = PLT_WORLD_BASE_TYPE_DESCRIPTOR_COUNT + type_descriptor_count;
	world->types = malloc(sizeof(Plt_Registered_Object_Type) * total_types);

	// Register base object types
	for (int i = 0; i < PLT_WORLD_BASE_TYPE_DESCRIPTOR_COUNT; ++i) {
		plt_world_register_object_type(world, base_type_descriptors[i]);
	}

	// Register custom object types
	for (int i = 0; i < type_descriptor_count; ++i) {
		plt_world_register_object_type(world, type_descriptors[i]);
	}

	// Check for duplicated object IDs
	for (int i = 0; i < world->type_count; ++i) {
		Plt_Object_Type_ID id = world->types[i].id;
		plt_assert(id != 0, "Registered object type can not be 0, this value is reserved for the 'none' type.\n");
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

	world->deferred_object_destroy_calls = malloc(sizeof(Plt_Object *) * object_storage_capacity);
	world->deferred_object_destroy_call_count = 0;
	world->is_updating = false;

	return world;
}

void plt_world_destroy(Plt_World **world) {
	// Destroy all objects
	Plt_Linked_List_Node *node = (*world)->object_list.root;
	while (node) {
		Plt_Object *object = node->data;
		Plt_Object_Private_Data *object_private = plt_world_get_object_private_data(*world, object);
		Plt_Linked_List_Node *next = node->next;
		if (object_private->parent == NULL) {
			plt_world_destroy_object(*world, &object, true);
		}
		node = next;
	}
	
	for (int i = 0; i < (*world)->type_count; ++i) {
		void *object_data = (*world)->types[i].object_entries;
		if (object_data) {
			free(object_data);
		}
	}
	free((*world)->object_storage_private_data);
	free((*world)->object_storage);
	free((*world)->deferred_object_destroy_calls);
	free((*world)->types);
	free(*world);
	*world = NULL;
}

Plt_Object *plt_world_get_object_at_path(Plt_World *world, const char *path) {
	unsigned int path_component_length;
	char path_component[128];
	bool is_last_component;
	bool is_go_to_parent;
	plt_object_path_get_component(path_component, path, &path_component_length, &is_last_component, &is_go_to_parent);
	plt_assert(!is_go_to_parent, "'..' in path is invalid, world has no parent.\n");
	
	Plt_Linked_List_Node *node = world->object_list.root;
	while (node) {
		Plt_Object *object = node->data;
		if (plt_world_get_object_parent(world, object) == NULL) {
			bool match = (strcmp(object->name, path_component) == 0);
			if (match) {
				if (is_last_component) {
					return object;
				} else {
					return plt_object_get_object_at_path(object, path + path_component_length + 1);
				}
			}
		}
		node = node->next;
	}
	
	return NULL;
}

unsigned int plt_world_get_object_index(Plt_World *world, Plt_Object *object) {
	return (object - world->object_storage);
}

Plt_Object *plt_world_get_object_at_index(Plt_World *world, unsigned int index) {
	return (Plt_Object *)(world->object_storage + index);
}

Plt_Object *plt_world_create_object(Plt_World *world, Plt_Object *parent, Plt_Object_Type_ID type, const char *name) {
	Plt_Linked_List_Node *node;
	plt_assert(world->object_count < world->object_storage_capacity, "World's object capacity has been exceeded.\n");

	unsigned int object_index;
	for (int i = 0; i < world->object_storage_capacity; ++i) {
		if (!world->object_storage_private_data[i].is_set) {
			object_index = i;
			world->object_storage_private_data[i].parent = parent;
			world->object_storage_private_data[i].children.root = NULL;
			break;
		}
	}

	Plt_Object *object = plt_world_get_object_at_index(world, object_index);
	object->world = world;
	object->type = type;
	object->name = name;
	object->transform = (Plt_Transform) {
		.translation = (Plt_Vector3f){0.0f, 0.0f, 0.0f},
		.rotation = (Plt_Quaternion){0.0f, 0.0f, 0.0f, 1.0f},
		.scale = (Plt_Vector3f){1.0f, 1.0f, 1.0f},
	};
	object->type_data = NULL;
	world->object_storage_private_data[object_index].is_set = true;
	world->object_storage_private_data[object_index].collision_count = 0;
	world->object_count++;

	// Add as child to parent
	if (parent) {
		unsigned int parent_index = plt_world_get_object_index(world, parent);
		Plt_Object_Private_Data *parent_private_data = &world->object_storage_private_data[parent_index];
		node = malloc(sizeof(Plt_Linked_List_Node));
		node->data = object;
		node->next = parent_private_data->children.root;
		parent_private_data->children.root = node;
	}
	
	node = malloc(sizeof(Plt_Linked_List_Node));
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
		if (registered_type->descriptor.data_size > 0) {
			memset(entry->data, 0, registered_type->descriptor.data_size);
			object->type_data = entry->data;
		}
		registered_type->object_entry_count++;
	}

	return object;
}

void plt_world_destroy_object(Plt_World *world, Plt_Object **object, bool including_children) {
	Plt_Linked_List_Node *node;
	unsigned int object_index = plt_world_get_object_index(world, *object);
	plt_assert(object_index < world->object_storage_capacity, "Object is outside of world's object capacity\n");

	if (world->is_updating) {
		// Defer object destruction until after the update is finished.
		world->deferred_object_destroy_calls[world->deferred_object_destroy_call_count++] = *object;
		*object = NULL;
		return;
	}
		
	if (world->object_storage_private_data[object_index].is_set) {
		world->object_storage_private_data[object_index].is_set = false;
		world->object_count--;
	}
	
	// Remove from object list
	node = world->object_list.root;
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
		Plt_Registered_Object_Type *type = plt_world_get_registered_object_type(world, (*object)->type);
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

	if (including_children) {
		// Remove children
		Plt_Object_Private_Data *object_private = plt_world_get_object_private_data(world, *object);
		node = object_private->children.root;
		while (node) {
			Plt_Object *child = node->data;
			plt_world_destroy_object(world, &child, true);
			Plt_Linked_List_Node *next = node->next;
			free(node);
			node = next;
		}
	}

	*object = NULL;
}

void plt_world_register_object_type(Plt_World *world, Plt_Object_Type_Descriptor descriptor) {
	unsigned int entry_stride = sizeof(Plt_Registered_Object_Type_Entry) + descriptor.data_size;

	world->types[world->type_count++] = (Plt_Registered_Object_Type) {
		.id = descriptor.id,
		.descriptor = descriptor,
		.object_entry_stride = entry_stride,
		.object_entries = malloc(entry_stride * world->object_storage_capacity),
		.object_entry_count = 0
	};
}

void plt_world_update_object_matrices_r(Plt_World *world, Plt_Object *object, Plt_Matrix4x4f parent_matrix) {
	Plt_Object_Private_Data *private_data = plt_world_get_object_private_data(world, object);
	private_data->parent_matrix = parent_matrix;

	if (private_data->children.root == NULL) {
		// Object has no children, stop recursing.
		return;
	}

	// Update child matrices
	Plt_Matrix4x4f child_parent_matrix = plt_matrix_multiply(parent_matrix, plt_transform_to_matrix(object->transform));
	Plt_Linked_List_Node *node = private_data->children.root;
	while (node != NULL) {
		Plt_Object *child = node->data;
		plt_world_update_object_matrices_r(world, child, child_parent_matrix);
		node = node->next;
	}
}

void plt_world_update_object_matrices(Plt_World *world) {
	// Get root level objects
	Plt_Linked_List_Node *node = world->object_list.root;
	while (node != NULL) {
		Plt_Object *object = node->data;
		if (plt_world_get_object_parent(world, object) == NULL) {
			plt_world_update_object_matrices_r(world, object, plt_matrix_identity());
		}
		node = node->next;
	}
}

void plt_world_update_object_collisions(Plt_World *world) {
	// Reset all collision counts to zero
	Plt_Linked_List_Node *node = world->object_list.root;
	while (node != NULL) {
		Plt_Object *object = node->data;
		plt_world_get_object_private_data(world, object)->collision_count = 0;
		node = node->next;
	}
	
	Plt_Registered_Object_Type *collider_type = plt_world_get_registered_object_type(world, Plt_Object_Type_Collider);
	plt_assert(collider_type, "The base collider object type is not registered in the world.\n");

	Plt_Registered_Object_Type_Entry *entry = (Plt_Registered_Object_Type_Entry *)collider_type->object_entries;
	for (unsigned int j = 0; j < collider_type->object_entry_count; ++j) {
		Plt_Object_Type_Collider_Data *collider_data = (Plt_Object_Type_Collider_Data *)entry->data;
		Plt_Object_Private_Data *object_private = plt_world_get_object_private_data(world, entry->object);
		
		Plt_Matrix4x4f object_matrix = plt_object_get_model_matrix(entry->object);

		// Check for collisions
		Plt_Registered_Object_Type_Entry *cmp_entry = (Plt_Registered_Object_Type_Entry *)collider_type->object_entries;
		for (unsigned int k = 0; k < collider_type->object_entry_count; ++k) {
			if (j != k) {
				Plt_Object_Type_Collider_Data *cmp_collider_data = (Plt_Object_Type_Collider_Data *)cmp_entry->data;
				Plt_Matrix4x4f cmp_object_matrix = plt_object_get_model_matrix(cmp_entry->object);
				
				// Check for collision
				bool collision = false;
				if ((collider_data->shape_type == Plt_Shape_Type_Box) && (cmp_collider_data->shape_type == Plt_Shape_Type_Box)) {
					collision = plt_collision_box_box(object_matrix, cmp_object_matrix, collider_data->box_shape, cmp_collider_data->box_shape);
				} else {
					plt_abort("Unsupported collision type: only box collisions are supported currently.\n");
				}
				
				if (collision) {
					// Push collision up through object parent hierarchy; all parent objects will receive a collision event.
					Plt_Object *o = entry->object;
					while (o) {
						Plt_Object_Private_Data *o_private = plt_world_get_object_private_data(world, o);
						if (o_private->collision_count < PLT_MAXIMUM_COLLISIONS_PER_OBJECT) {
							o_private->collisions[o_private->collision_count++] = cmp_entry->object;
						}
						o = plt_world_get_object_parent(world, o);
					}
				}
			}
			
			// Move to next comparison collider
			cmp_entry = (Plt_Registered_Object_Type_Entry *)(((char *)cmp_entry) + collider_type->object_entry_stride);
		}
		
		// Move to next collider
		entry = (Plt_Registered_Object_Type_Entry *)(((char *)entry) + collider_type->object_entry_stride);
	}
}

void plt_world_destroy_deferred_objects(Plt_World *world) {
	for (unsigned int i = 0; i < world->deferred_object_destroy_call_count; ++i) {
		plt_world_destroy_object(world, &world->deferred_object_destroy_calls[i], true);
	}
	world->deferred_object_destroy_call_count = 0;
}

void plt_world_update_begin(Plt_World *world) {
	world->is_updating = true;
}

void plt_world_update_finish(Plt_World *world) {
	world->is_updating = false;
	plt_world_destroy_deferred_objects(world);
}

void plt_world_update(Plt_World *world, Plt_Frame_State frame_state) {
	plt_world_update_begin(world);
	plt_world_update_object_matrices(world);
	plt_world_update_object_collisions(world);

	for (unsigned int i = 0; i < world->type_count; ++i) {
		Plt_Registered_Object_Type type = world->types[i];
		void (*update_func)(Plt_Object *object, void *type_data, Plt_Frame_State frame_state) = type.descriptor.update;
		if (!update_func) {
			continue;
		}
		
		Plt_Registered_Object_Type_Entry *entry = (Plt_Registered_Object_Type_Entry *)type.object_entries;
		for (unsigned int j = 0; j < type.object_entry_count; ++j) {
			update_func(entry->object, entry->data, frame_state);
			
			entry = (Plt_Registered_Object_Type_Entry *)(((char *)entry) + type.object_entry_stride);
		}
	}

	plt_world_update_finish(world);
}

void plt_world_render_scene(Plt_World *world, Plt_Frame_State frame_state, Plt_Renderer *renderer) {
	plt_world_update_begin(world);
	plt_world_update_object_matrices(world);

	// Get camera object
	Plt_Object *camera = NULL;
	Plt_Registered_Object_Type *type = plt_world_get_registered_object_type(world, Plt_Object_Type_Camera);
	if (type->object_entry_count > 0) {
		camera = ((Plt_Registered_Object_Type_Entry *)type->object_entries)[0].object;
	}
	if (!camera) {
		printf("No camera in world.\n");
		plt_renderer_clear(renderer, plt_color8_make(0, 0, 0, 255));
		return;
	}

	Plt_Vector2i viewport = { renderer->framebuffer.width, renderer->framebuffer.height };
	plt_renderer_set_view_matrix(renderer, plt_object_type_camera_get_view_matrix(camera));
	plt_renderer_set_projection_matrix(renderer, plt_object_type_camera_get_projection_matrix(camera, viewport));

	for (unsigned int i = 0; i < world->type_count; ++i) {
		Plt_Registered_Object_Type type = world->types[i];
		void (*render_scene_func)(Plt_Object *object, void *type_data, Plt_Frame_State state, Plt_Renderer *renderer) = type.descriptor.render_scene;
		if (!render_scene_func) {
			continue;
		}
		
		Plt_Registered_Object_Type_Entry *entry = (Plt_Registered_Object_Type_Entry *)type.object_entries;
		for (unsigned int j = 0; j < type.object_entry_count; ++j) {
			render_scene_func(entry->object, entry->data, frame_state, renderer);
			
			entry = (Plt_Registered_Object_Type_Entry *)(((char *)entry) + type.object_entry_stride);
		}
	}
	
	plt_world_update_finish(world);
}

void plt_world_render_ui(Plt_World *world, Plt_Frame_State frame_state, Plt_Renderer *renderer) {
	plt_world_update_begin(world);

	for (unsigned int i = 0; i < world->type_count; ++i) {
		Plt_Registered_Object_Type type = world->types[i];
		void (*render_ui_func)(Plt_Object *object, void *type_data, Plt_Frame_State state, Plt_Renderer *renderer) = type.descriptor.render_ui;
		if (!render_ui_func) {
			continue;
		}
		
		Plt_Registered_Object_Type_Entry *entry = (Plt_Registered_Object_Type_Entry *)type.object_entries;
		for (unsigned int j = 0; j < type.object_entry_count; ++j) {
			render_ui_func(entry->object, entry->data, frame_state, renderer);
			
			entry = (Plt_Registered_Object_Type_Entry *)(((char *)entry) + type.object_entry_stride);
		}
	}
	
	plt_world_update_finish(world);
}

Plt_Object_Private_Data *plt_world_get_object_private_data(Plt_World *world, Plt_Object *object) {
	unsigned int object_index = plt_world_get_object_index(world, object);
	return &world->object_storage_private_data[object_index];
}

Plt_Registered_Object_Type *plt_world_get_registered_object_type(Plt_World *world, Plt_Object_Type_ID id) {
	for (unsigned int i = 0; i < world->type_count; ++i) {
		if (world->types[i].id == id) {
			return &world->types[i];
		}
	}
	return NULL;
}

Plt_Linked_List *plt_world_get_object_children(Plt_World *world, Plt_Object *object) {
	return &plt_world_get_object_private_data(world, object)->children;
}

Plt_Object *plt_world_get_object_parent(Plt_World *world, Plt_Object *object) {
	return plt_world_get_object_private_data(world, object)->parent;
}

void plt_world_set_object_parent(Plt_World *world, Plt_Object *object, Plt_Object *parent) {
	Plt_Linked_List_Node *node;
	Plt_Linked_List_Node *object_node = NULL;
	
	Plt_Object_Private_Data *object_private = plt_world_get_object_private_data(world, object);
	
	if (object_private->parent == parent) {
		// Object is already parented to the given parent object.
		return;
	}
	
	if (object_private->parent) {
		Plt_Object_Private_Data *parent_private = plt_world_get_object_private_data(world, parent);

		// Remove from children of original parent
		node = parent_private->children.root;
		if (node->data == object) {
			parent_private->children.root = node->next;
			node->next = NULL;
			object_node = node;
		} else {
			while (node->next) {
				if (node->next->data == object) {
					object_node = node->next;
					node->next = node->next->next;
					object_node->next = NULL;
				}
				node = node->next;
			}
		}
	} else {
		object_node = malloc(sizeof(Plt_Linked_List_Node));
		object_node->data = object;
		object_node->next = NULL;
	}
	
	if (parent) {
		// Add as child in new parent object
		Plt_Object_Private_Data *parent_private = plt_world_get_object_private_data(world, parent);
		object_node->next = parent_private->children.root;
		parent_private->children.root = object_node;
	} else {
		free(object_node);
	}
	
	object_private->parent = parent;
}

Plt_Matrix4x4f plt_world_get_object_parent_matrix(Plt_World *world, Plt_Object *object) {
	return plt_world_get_object_private_data(world, object)->parent_matrix;
}

Plt_Object **plt_world_get_object_collisions(Plt_World *world, Plt_Object *object, unsigned int *collision_count) {
	Plt_Object_Private_Data *data = plt_world_get_object_private_data(world, object);
	*collision_count = data->collision_count;
	return data->collisions;
}
