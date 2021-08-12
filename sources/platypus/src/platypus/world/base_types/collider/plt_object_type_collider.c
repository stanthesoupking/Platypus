#include "plt_object_type_collider.h"

#include "platypus/base/plt_defines.h"
#include <stdlib.h>

#if PLT_DEBUG_COLLIDERS
void _collider_type_render(Plt_Object *object, void *type_data, Plt_Frame_State state, Plt_Renderer *renderer) {
	Plt_Object_Type_Collider_Data *data = (Plt_Object_Type_Collider_Data *)type_data;

	if (data->shape_type == Plt_Shape_Type_Box) {
		Plt_Mesh *mesh = plt_mesh_create_cube(data->box_shape.size);
		plt_renderer_bind_texture(renderer, NULL);
		plt_renderer_set_lighting_model(renderer, Plt_Lighting_Model_Unlit);
		plt_renderer_set_render_color(renderer, plt_color8_make(0, 255, 0, 255));
		plt_renderer_set_model_matrix(renderer, plt_object_get_model_matrix(object));
		plt_renderer_set_primitive_type(renderer, Plt_Primitive_Type_Line);
		plt_renderer_set_point_size(renderer, 4);
		plt_renderer_draw_mesh(renderer, mesh);
		plt_mesh_destroy(&mesh);
	}
}
#endif

Plt_Object_Type_Descriptor plt_object_type_collider_get_descriptor() {
	return (Plt_Object_Type_Descriptor) {
		.id = Plt_Object_Type_Collider,
		.data_size = sizeof(Plt_Object_Type_Collider_Data),
		.update = NULL,

		#if PLT_DEBUG_COLLIDERS
		.render_scene = _collider_type_render,
		#else
		.render_scene = NULL,
		#endif

		.render_ui = NULL
	};
}