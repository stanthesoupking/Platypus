#pragma once

#include "platypus/platypus.h"
#include "platypus/base/plt_linked_list.h"
#include "plt_object.h"

typedef struct Plt_Registered_Object_Type_Entry {
	Plt_Object *object;
	char data[];
} Plt_Registered_Object_Type_Entry;

typedef struct Plt_Registered_Object_Type {
	Plt_Object_Type_ID id;
	Plt_Object_Type_Descriptor descriptor;
	unsigned int object_entry_stride;
	unsigned int object_entry_count;
	char *object_entries;
} Plt_Registered_Object_Type;

typedef struct Plt_Object_Private_Data {
	bool is_set;

	bool valid_matrices;
	Plt_Matrix4x4f parent_matrix;

	Plt_Object *parent;
	Plt_Linked_List children;
} Plt_Object_Private_Data;

typedef struct Plt_World {
	Plt_Linked_List object_list;

	unsigned int object_count;
	Plt_Object *object_storage;
	Plt_Object_Private_Data *object_storage_private_data;
	unsigned int object_storage_capacity;

	unsigned int type_count;
	Plt_Registered_Object_Type *types;
} Plt_World;

void plt_world_update(Plt_World *world, Plt_Frame_State frame_state);
void plt_world_render(Plt_World *world, Plt_Frame_State frame_state, Plt_Renderer *renderer);

Plt_Object_Private_Data *plt_world_get_object_private_data(Plt_World *world, Plt_Object *object);
Plt_Object *plt_world_get_object_parent(Plt_World *world, Plt_Object *object);
Plt_Matrix4x4f plt_world_get_object_parent_matrix(Plt_World *world, Plt_Object *object);