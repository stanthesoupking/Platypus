#include "plt_world.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "platypus/world/base_components/billboard_renderer/plt_component_billboard_renderer.h"
#include "platypus/world/base_components/camera/plt_component_camera.h"
#include "platypus/world/base_components/collider/plt_component_collider.h"
#include "platypus/world/base_components/flying_camera_controller/plt_component_flying_camera_controller.h"
#include "platypus/world/base_components/mesh_renderer/plt_component_mesh_renderer.h"

Plt_World *plt_world_create() {
	Plt_World *world = malloc(sizeof(Plt_World));

	world->current_entity_id = 1;
	world->current_component_id = 1;
	world->deferred_destroy_count = 0;
	world->entity_count = 0;
	world->component_count = 0;

	world->is_updating = false;
	
	plt_register_billboard_renderer_component(world);
	plt_register_camera_component(world);
	plt_register_collider_component(world);
	plt_register_flying_camera_component(world);
	plt_register_mesh_renderer_component(world);
	
	return world;
}

void plt_world_destroy(Plt_World *world) {
	for (unsigned int i = 0; i < world->component_count; ++i) {
		free(world->components[i].table.entries);
	}
	free(world);
}

void plt_world_update_begin(Plt_World *world) {
	world->is_updating = true;
}

void plt_world_update_finish(Plt_World *world) {
	world->is_updating = false;

	// Execute deferred entity destroy calls
	for (unsigned int i = 0; i < world->deferred_destroy_count; ++i) {
		plt_world_destroy_entity(world, world->deferred_destroy[i]);
	}
	world->deferred_destroy_count = 0;
}

void plt_world_update(Plt_World *world, Plt_Frame_State state) {
	plt_world_update_begin(world);

	for (unsigned int i = 0; i < world->component_count; ++i) {
		Plt_Component component = world->components[i];
		if (!component.update) {
			continue;
		}

		Plt_Component_Table_Entry *entry = component.table.entries;
		for (unsigned int j = 0; j < component.table.entry_count; ++j) {
			//printf("Updating entity %d(%s)\n", entry->entity_id, component.name);
			component.update(world, entry->entity_id, entry->instance_data, state);
			entry = (Plt_Component_Table_Entry *)(((char *)entry) + sizeof(Plt_Component_Table_Entry) + component.data_size);
		}
	}

	plt_world_update_finish(world);
}

void plt_world_render_scene(Plt_World *world, Plt_Frame_State state, Plt_Renderer *renderer) {
	plt_world_update_begin(world);
	
	// Update camera
	Plt_Entity_ID camera;
	if (plt_world_get_entity_with_component(world, PLT_COMPONENT_CAMERA, &camera)) {
		plt_renderer_set_view_matrix(renderer, plt_component_camera_get_view_matrix(world, camera));
		plt_renderer_set_projection_matrix(renderer, plt_component_camera_get_projection_matrix(world, camera, plt_renderer_get_framebuffer_size(renderer)));
	}

	for (unsigned int i = 0; i < world->component_count; ++i) {		
		Plt_Component component = world->components[i];
		if (!component.render_scene) {
			continue;
		}

		Plt_Component_Table_Entry *entry = component.table.entries;
		for (unsigned int j = 0; j < component.table.entry_count; ++j) {
			//printf("Updating entity %d(%s)\n", entry->entity_id, component.name);
			component.render_scene(world, entry->entity_id, entry->instance_data, state, renderer);
			entry = (Plt_Component_Table_Entry *)(((char *)entry) + sizeof(Plt_Component_Table_Entry) + component.data_size);
		}
	}

	plt_world_update_finish(world);
}

void plt_world_render_ui(Plt_World *world, Plt_Frame_State state, Plt_Renderer *renderer) {
	plt_world_update_begin(world);

	for (unsigned int i = 0; i < world->component_count; ++i) {
		Plt_Component component = world->components[i];
		if (!component.render_ui) {
			continue;
		}

		Plt_Component_Table_Entry *entry = component.table.entries;
		for (unsigned int j = 0; j < component.table.entry_count; ++j) {
			//printf("Updating entity %d(%s)\n", entry->entity_id, component.name);
			component.render_ui(world, entry->entity_id, entry->instance_data, state, renderer);
			entry = (Plt_Component_Table_Entry *)(((char *)entry) + sizeof(Plt_Component_Table_Entry) + component.data_size);
		}
	}

	plt_world_update_finish(world);
}

Plt_Entity *plt_world_get_entity(Plt_World *world, Plt_Entity_ID entity_id) {
	for (unsigned int i = 0; i < world->entity_count; ++i) {
		if (world->entities[i].id == entity_id) {
			return &world->entities[i];
		}
	}
	return NULL;
}

Plt_Component *plt_world_get_component_with_name(Plt_World *world, const char *component_name) {
	for (unsigned int i = 0; i < world->component_count; ++i) {
		if (strcmp(world->components[i].name, component_name) == 0) {
			return &world->components[i];
		}
	}
	return NULL;
}

Plt_Component *plt_world_get_component_with_id(Plt_World *world, Plt_Component_ID component_id) {
	for (unsigned int i = 0; i < world->component_count; ++i) {
		if (world->components[i].id == component_id) {
			return &world->components[i];
		}
	}
	return NULL;
}

Plt_Entity_ID plt_world_create_entity(Plt_World *world, const char *name, Plt_Entity_ID parent_id) {
	if (world->entity_count >= PLT_WORLD_ENTITY_CAPACITY) {
		return PLT_ENTITY_ID_NONE;
	}

	Plt_Entity entity = {
		.id = world->current_entity_id++,
		.parent = parent_id,
		.components = 0,
		.name = name,
		.transform = plt_transform_create(plt_vector3f_make(0, 0, 0), plt_quaternion_create_from_euler(plt_vector3f_make(0, 0, 0)), plt_vector3f_make(1, 1, 1))
	};

	world->entities[world->entity_count++] = entity;
	
	return entity.id;
}

void plt_world_destroy_entity(Plt_World *world, Plt_Entity_ID entity_id) {
	if (world->is_updating) {
		// Defer entity destruction until the end of the next update tick
		world->deferred_destroy[world->deferred_destroy_count++] = entity_id;
		return;
	}

	Plt_Entity *entity = NULL;
	unsigned int index = 0;
	for (unsigned int i = 0; i < world->entity_count; ++i) {
		if (world->entities[i].id == entity_id) {
			index = i;
			entity = &world->entities[i];
			break;
		}
	}

	if (entity) {
		// Remove all components
		for (unsigned int i = 0; i < sizeof(entity->components) * 8; ++i) {
			if ((entity->components & (1 << i)) > 0) {
				Plt_Component *component = plt_world_get_component_with_id(world, i);
				plt_world_entity_remove_component(world, entity_id, component->name);
			}
		}

		for (unsigned int i = index; i < world->entity_count - 1; ++i) {
			world->entities[i] = world->entities[i + 1];
		}
		world->entity_count--;
	}
}

void plt_world_entity_add_component(Plt_World *world, Plt_Entity_ID entity_id, const char *component_name) {
	Plt_Entity *entity = plt_world_get_entity(world, entity_id);
	if (!entity) {
		return;
	}

	Plt_Component *component = plt_world_get_component_with_name(world, component_name);
	if (!component) {
		printf("Error: Failed finding component with name: '%s'.\n", component_name);
		abort();
	}

	if ((entity->components & (1 << component->id)) != 0) {
		// Entity already has component
		return;
	}

	Plt_Component_Table_Entry *entry = (Plt_Component_Table_Entry *)((char *)component->table.entries + (component->table.entry_count++ * (component->data_size + sizeof(Plt_Component_Table_Entry))));
	entry->entity_id = entity_id;
	memset(entry->instance_data, 0, component->data_size);
	
	entity->components |= 1 << component->id;

	if (component->init) {
		component->init(world, entity_id, entry->instance_data);
	}
}

void plt_world_entity_remove_component(Plt_World *world, Plt_Entity_ID entity_id, const char *component_name) {
	Plt_Entity *entity = plt_world_get_entity(world, entity_id);
	if (!entity) {
		printf("Error: Failed finding entity with ID: '%d'.\n", entity_id);
		abort();
	}

	Plt_Component *component = plt_world_get_component_with_name(world, component_name);
	if (!component) {
		printf("Error: Failed finding component with name: '%s'.\n", component_name);
		abort();
	}

	unsigned long long component_mask = (1 << component->id);
	if ((entity->components & component_mask) == 0) {
		return;
	}

	entity->components &= ~component_mask;

	bool found = false;
	unsigned int index = 0;
	for (unsigned int i = 0; i < component->table.entry_count; ++i) {
		if (component->table.entries->entity_id == entity_id) {
			index = i;
			found = true;
			break;
		}
	}

	if (found) {
		for (unsigned int i = index; i < component->table.entry_count - 1; ++i) {
			component->table.entries[i] = component->table.entries[i + 1];
		}
		component->table.entry_count--;
	}
}

Plt_Matrix4x4f plt_world_entity_get_model_matrix(Plt_World *world, Plt_Entity_ID entity_id) {
	Plt_Entity *entity = plt_world_get_entity(world, entity_id);
	if (!entity) {
		return plt_matrix_identity();
	}
	
	if (entity->parent == PLT_ENTITY_ID_NONE) {
		return plt_transform_to_matrix(entity->transform);
	} else {
		return plt_matrix_multiply(plt_world_entity_get_model_matrix(world, entity->parent), plt_transform_to_matrix(entity->transform));
	}
}

Plt_Vector3f plt_entity_get_forward(Plt_World *world, Plt_Entity_ID entity_id) {
	Plt_Transform transform = plt_world_entity_get_transform(world, entity_id);
	return plt_quaternion_rotate_vector(transform.rotation, plt_vector3f_make(0, 0, -1));
}

Plt_Vector3f plt_entity_get_right(Plt_World *world, Plt_Entity_ID entity_id) {
	Plt_Transform transform = plt_world_entity_get_transform(world, entity_id);
	return plt_quaternion_rotate_vector(transform.rotation, plt_vector3f_make(1, 0, 0));
}

Plt_Vector3f plt_entity_get_up(Plt_World *world, Plt_Entity_ID entity_id) {
	Plt_Transform transform = plt_world_entity_get_transform(world, entity_id);
	return plt_quaternion_rotate_vector(transform.rotation, plt_vector3f_make(0, 1, 0));
}

Plt_Transform plt_world_entity_get_transform(Plt_World *world, Plt_Entity_ID entity_id) {
	Plt_Entity *entity = plt_world_get_entity(world, entity_id);
	if (entity) {
		return entity->transform;
	} else {
		return (Plt_Transform){};
	}
}

void plt_world_entity_set_transform(Plt_World *world, Plt_Entity_ID entity_id, Plt_Transform transform) {
	Plt_Entity *entity = plt_world_get_entity(world, entity_id);
	if (entity) {
		entity->transform = transform;
	}
}

const char *plt_world_entity_get_name(Plt_World *world, Plt_Entity_ID entity_id) {
	Plt_Entity *entity = plt_world_get_entity(world, entity_id);
	if (entity) {
		return entity->name;
	} else {
		return NULL;
	}
}

void plt_world_entity_set_name(Plt_World *world, Plt_Entity_ID entity_id, const char *name) {
	Plt_Entity *entity = plt_world_get_entity(world, entity_id);
	if (entity) {
		entity->name = name;
	}
}

Plt_Entity_ID plt_world_entity_get_parent(Plt_World *world, Plt_Entity_ID entity_id) {
	Plt_Entity *entity = plt_world_get_entity(world, entity_id);
	return entity ? entity->parent : PLT_ENTITY_ID_NONE;
}

void plt_world_entity_set_parent(Plt_World *world, Plt_Entity_ID entity_id, Plt_Entity_ID parent_id) {
	Plt_Entity *entity = plt_world_get_entity(world, entity_id);
	if (entity) {
		entity->parent = parent_id;
	}
}

bool plt_world_get_entity_with_component(Plt_World *world, const char *component_name, Plt_Entity_ID *found) {
	Plt_Component *component = plt_world_get_component_with_name(world, component_name);
	if (component) {
		if (component->table.entry_count > 0) {
			*found = component->table.entries[0].entity_id;
			return true;
		}
	}
	return false;
}

void plt_world_get_entities_with_component(Plt_World *world, const char *component_name, Plt_Entity_ID *result_entities, unsigned int *result_entity_count) {
	*result_entity_count = 0;

	Plt_Component *component = plt_world_get_component_with_name(world, component_name);
	if (component) {
		for (unsigned int i = 0; i < component->table.entry_count; ++i) {
			result_entities[i] = component->table.entries[i].entity_id;
		}
	}
}


void plt_world_register_component(Plt_World *world, const char *component_name, unsigned int data_size, void (*init)(Plt_World *world, Plt_Entity_ID entity_id, void *instance_data), void (*update)(Plt_World *world, Plt_Entity_ID entity_id, void *instance_data, Plt_Frame_State state), void (*render_scene)(Plt_World *world, Plt_Entity_ID entity_id, void *instance_data, Plt_Frame_State state, Plt_Renderer *renderer), void (*render_ui)(Plt_World *world, Plt_Entity_ID entity_id, void *instance_data, Plt_Frame_State state, Plt_Renderer *renderer)) {
	// Verify that no components with this name already exist
	for (unsigned int i = 0; i < world->component_count; ++i) {
		if (strcmp(world->components[i].name, component_name) == 0) {
			printf("Error: A component already exists with the name: '%s'.\n", component_name);
			abort();
		}
	}

	Plt_Component component = {
		.name = component_name,
		.id = world->current_component_id++,
		.data_size = data_size,
		.init = init,
		.update = update,
		.render_scene = render_scene,
		.render_ui = render_ui,
		.table = {
			.entry_count = 0,
			.entries = malloc(PLT_WORLD_ENTITY_CAPACITY * (data_size + sizeof(Plt_Component_Table_Entry)))
		}
	};

	world->components[world->component_count++] = component;
}

void *plt_world_get_component_instance_data(Plt_World *world, Plt_Entity_ID entity_id, const char *component_name) {
	Plt_Component *component = plt_world_get_component_with_name(world, component_name);
	if (!component) {
		return NULL;
	}
	
	Plt_Component_Table_Entry *entry = component->table.entries;
	for (unsigned int i = 0; i < component->table.entry_count; ++i) {
		if (entry->entity_id == entity_id) {
			return entry->instance_data;
		}
		entry = (Plt_Component_Table_Entry *)(((char *)entry) + sizeof(Plt_Component_Table_Entry) + component->data_size);
	}
	
	return NULL;
}
