#pragma once

#include "platypus/platypus.h"

typedef struct Plt_Object_Ref {
	Plt_World *world;
	unsigned int world_object_storage_index;
} Plt_Object_Ref;

typedef struct Plt_Object {
	Plt_Object_Ref *ref;
} Plt_Object;

typedef struct Plt_Object_Data {
	Plt_Object_Type_ID type;

	const char *name;
	Plt_Transform transform;
	Plt_Object *parent;
} Plt_Object_Data;