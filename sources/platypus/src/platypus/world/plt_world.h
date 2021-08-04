#pragma once

#include "platypus/platypus.h"
#include "platypus/base/plt_linked_list.h"
#include "plt_object.h"

typedef struct Plt_Registered_Object_Type {
	bool is_set;
	Plt_Object_Type_Descriptor descriptor;
} Plt_Registered_Object_Type;

#define PLT_WORLD_MAX_OBJECT_TYPES 2048

typedef struct Plt_Object_Private_Data {
	bool is_set;

	bool valid_matrices;
	Plt_Matrix4x4f parent_matrix;
} Plt_Object_Private_Data;

typedef struct Plt_World {
	Plt_Linked_List object_ref_list;

	unsigned int object_count;
	char *object_storage;
	Plt_Object_Private_Data *object_storage_private_data;
	unsigned int object_storage_capacity;

	unsigned int object_size;
	unsigned int inclusive_object_type_data_size;
	Plt_Registered_Object_Type registered_object_types[PLT_WORLD_MAX_OBJECT_TYPES];
} Plt_World;

void plt_world_update(Plt_World *world);
void plt_world_render(Plt_World *world, Plt_Renderer *renderer);

Plt_Matrix4x4f plt_world_get_object_parent_matrix(Plt_World *world, Plt_Object *object);