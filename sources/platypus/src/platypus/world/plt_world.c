#include "plt_world.h"

#include <stdlib.h>
#include "platypus/base/macros.h"

Plt_World *plt_world_create(unsigned int object_storage_capacity) {
	plt_assert(object_storage_capacity > 0, "Object storage capacity must be at least 1");

	Plt_World *world = malloc(sizeof(Plt_World));

	world->object_ref_list.root = NULL;

	world->object_count = 0;
	world->object_storage_capacity = object_storage_capacity;
	world->object_storage = malloc(sizeof(Plt_Object_Data) * object_storage_capacity);

	return world;
}

void plt_world_destroy(Plt_World **world) {
	free((*world)->object_storage);
	free(*world);
	*world = NULL;
}

Plt_Object_Ref *plt_world_create_object(Plt_World *world, Plt_Object_Type_ID type, const char *name) {
	unsigned int object_index = world->object_count++;

	Plt_Object_Data *object_data = &world->object_storage[object_index];
	object_data->type = type;
	object_data->name = name;
	object_data->parent = NULL;
	object_data->transform = (Plt_Transform) {
		.translation = (Plt_Vector3f){0.0f, 0.0f, 0.0f},
		.rotation = (Plt_Vector3f){0.0f, 0.0f, 0.0f},
		.scale = (Plt_Vector3f){1.0f, 1.0f, 1.0f},
	};

	Plt_Object_Ref *object_ref = malloc(sizeof(Plt_Object_Ref));
	object_ref->world = world;
	object_ref->world_object_storage_index = object_index;

	Plt_Linked_List_Node *ref_list_node = malloc(sizeof(Plt_Linked_List_Node));
	ref_list_node->data = object_ref;
	ref_list_node->next = world->object_ref_list.root;
	world->object_ref_list.root = ref_list_node;

	return object_ref;
}

void plt_world_destroy_object(Plt_World *world, Plt_Object_Ref **object_ref) {
	Plt_Linked_List_Node *node;

	world->object_count--;
	for(unsigned int i = (*object_ref)->world_object_storage_index; i < world->object_count; ++i) {
		world->object_storage[i] = world->object_storage[i + 1];
	}

	// Remove destroyed object ref
	node = world->object_ref_list.root;
	if (node->data == (*object_ref)) {
		world->object_ref_list.root = node->next;
		free(node);
	} else {
		while (node->next != NULL) {
			if (node->next->data == (*object_ref)) {
				Plt_Linked_List_Node *d = node->next;
				node->next = node->next->next;
				free(d);
				break;
			} else {
				node = node->next;
			}
		}
	}

	// Update object refs
	node = world->object_ref_list.root;
	while (node != NULL) {
		Plt_Object_Ref *r = node->data;
		if (r->world_object_storage_index > ((*object_ref)->world_object_storage_index)) {
			--r->world_object_storage_index;
		}
		node = node->next;
	}

	free(*object_ref);
	*object_ref = NULL;
}

Plt_Object_Data *plt_world_dereference_object(Plt_Object_Ref *object_ref) {
	return &object_ref->world->object_storage[object_ref->world_object_storage_index];
}
