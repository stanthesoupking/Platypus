#include "plt_object.h"

#include "plt_world.h"

Plt_Object *plt_object_create(Plt_World *world, Plt_Object *parent, Plt_Object_Type_ID type, const char *name) {
	plt_assert(world, "An object can't be created without a world.");
	if (parent) {
		plt_assert(parent->world == world, "Object parents must be in the same world as the child object.\n");
	}
	return plt_world_create_object(world, parent, type, name);
}

void plt_object_destroy(Plt_Object **object) {
	plt_world_destroy_object((*object)->world, object, true);
}

void plt_object_path_get_component(char *dest_component, const char *src_path, unsigned int *component_length, bool *is_last_component, bool *is_go_to_parent) {
	*component_length = 0;
	*is_go_to_parent = false;
	char c = *src_path;
	
	// Ignore leading '/'
	if (c == '/') {
		c = *(++src_path);
	}
		
	while ((c != '\0') && (c != '/')) {
		dest_component[(*component_length)++] = c;
		c = *(++src_path);
	}
	dest_component[*component_length] = '\0';
	*is_last_component = (c != '/');
	
	// Check if component is '..' or go to parent
	if ((*component_length == 2) && (strcmp(dest_component, "..") == 0)) {
		*is_go_to_parent = true;
	}
}

Plt_Object *plt_object_get_object_at_path(Plt_Object *object, const char *path) {
	Plt_Linked_List *children = plt_world_get_object_children(object->world, object);
	Plt_Linked_List_Node *node = children->root;

	unsigned int path_component_length;
	char path_component[128];
	bool is_last_component;
	bool is_go_to_parent;
	plt_object_path_get_component(path_component, path, &path_component_length, &is_last_component, &is_go_to_parent);
	if ((path_component_length == 1) && (path_component[0] == '.')) {
		if (is_last_component) {
			return object;
		} else {
			return plt_object_get_object_at_path(object, path + path_component_length + 1);
		}
	}
	
	if (is_go_to_parent) {
		return plt_object_get_parent(object);
	}

	while (node) {
		Plt_Object *child = node->data;

		bool match = (strcmp(child->name, path_component) == 0);
		if (match) {
			if (is_last_component) {
				return child;
			} else {
				return plt_object_get_object_at_path(child, path + path_component_length + 1);
			}
		}

		node = node->next;
	}
	
	return NULL;
}

Plt_Object *plt_object_get_child_object_of_type(Plt_Object *object, Plt_Object_Type_ID type) {
	Plt_Linked_List *children = plt_world_get_object_children(object->world, object);
	Plt_Linked_List_Node *node = children->root;

	while (node) {
		Plt_Object *child = node->data;
		if (child->type == type) {
			return child;
		} else {
			Plt_Object *found_in_child = plt_object_get_child_object_of_type(child, type);
			if (found_in_child) {
				return found_in_child;
			}
		}
		node = node->next;
	}
	return NULL;
}

Plt_Object *plt_object_get_root(Plt_Object *object) {
	Plt_Object *parent = plt_object_get_parent(object);
	if (parent == NULL) {
		return object;
	} else {
		return plt_object_get_parent(parent);
	}
}

Plt_Object *plt_object_get_parent(Plt_Object *object) {
	return plt_world_get_object_parent(object->world, object);
}

void plt_object_set_parent(Plt_Object *object, Plt_Object *parent) {
	plt_assert(object->world == parent->world, "Object parents must be in the same world as the child object.\n");
	plt_world_set_object_parent(object->world, object, parent);
}

Plt_Matrix4x4f plt_object_get_model_matrix(Plt_Object *object) {
	Plt_Matrix4x4f parent_matrix = plt_world_get_object_parent_matrix(object->world, object);
	return plt_matrix_multiply(parent_matrix, plt_transform_to_matrix(object->transform));
}

Plt_Object **plt_object_get_collisions(Plt_Object *object, unsigned int *collision_count) {
	return plt_world_get_object_collisions(object->world, object, collision_count);
}

Plt_Vector3f plt_object_get_forward(Plt_Object *object) {
	Plt_Vector4f v = { 0.0f, 0.0f, -1.0f, 0.0f };
	Plt_Matrix4x4f model = plt_object_get_model_matrix(object);
	Plt_Vector4f result = plt_matrix_multiply_vector4f(model, v);
	return (Plt_Vector3f){ result.x, result.y, result.z };
}

Plt_Vector3f plt_object_get_up(Plt_Object *object) {
	Plt_Vector4f v = { 0.0f, 1.0f, 0.0f, 0.0f };
	Plt_Matrix4x4f model = plt_object_get_model_matrix(object);
	Plt_Vector4f result = plt_matrix_multiply_vector4f(model, v);
	return (Plt_Vector3f){ result.x, result.y, result.z };
}

Plt_Vector3f plt_object_get_right(Plt_Object *object) {
	Plt_Vector4f v = { 1.0f, 0.0f, 0.0f, 0.0f };
	Plt_Matrix4x4f model = plt_object_get_model_matrix(object);
	Plt_Vector4f result = plt_matrix_multiply_vector4f(model, v);
	return (Plt_Vector3f){ result.x, result.y, result.z };
}
