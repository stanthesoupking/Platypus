#include "plt_object_type_mesh_renderer.h"

void _mesh_renderer_type_render(Plt_Object *object, void *type_data, Plt_Renderer *renderer) {
	Plt_Object_Type_Mesh_Renderer_Data *mesh_type_data = (Plt_Object_Type_Mesh_Renderer_Data *)type_data;
	
	if (!mesh_type_data->mesh) {
		return;
	}
	
	plt_renderer_set_model_matrix(renderer, plt_object_get_model_matrix(object));
	plt_renderer_bind_texture(renderer, mesh_type_data->texture);
	plt_renderer_set_render_color(renderer, mesh_type_data->color);
	plt_renderer_set_primitive_type(renderer, Plt_Primitive_Type_Triangle);
	plt_renderer_draw_mesh(renderer, mesh_type_data->mesh);
}