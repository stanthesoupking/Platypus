#pragma once

#include "platypus/platypus.h"
#include "platypus/base/plt_linked_list.h"
#include "plt_object.h"

typedef struct Plt_World {
	Plt_Linked_List object_ref_list;

	unsigned int object_count;
	Plt_Object_Data *object_storage;
	unsigned int object_storage_capacity;
} Plt_World;

Plt_World *plt_world_create(unsigned int object_storage_capacity);
void plt_world_destroy(Plt_World **world);

Plt_Object_Ref *plt_world_create_object(Plt_World *world, Plt_Object_Type_ID type, const char *name);
void plt_world_destroy_object(Plt_World *world, Plt_Object_Ref **object_ref);

Plt_Object_Data *plt_world_dereference_object(Plt_Object_Ref *object_ref);