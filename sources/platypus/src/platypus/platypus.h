#ifndef PLATYPUS_H
#define PLATYPUS_H

#include <stdbool.h>

// MARK: Math

#define PLT_PI 3.1415926535897932384626

typedef struct Plt_Vector2i {
	int x, y;
} Plt_Vector2i;

typedef struct Plt_Vector3i {
	int x, y, z;
} Plt_Vector3i;

typedef struct Plt_Vector4i {
	int x, y, z, w;
} Plt_Vector4i;

typedef struct Plt_Vector2f {
	float x, y;
} Plt_Vector2f;

typedef struct Plt_Vector3f {
	float x, y, z;
} Plt_Vector3f;

typedef struct Plt_Vector4f {
	float x, y, z, w;
} Plt_Vector4f;

typedef struct Plt_Matrix4x4f {
	float columns[4][4];
} Plt_Matrix4x4f;

// Values defined in row-major format
Plt_Matrix4x4f plt_matrix_create(float v[4][4]);
Plt_Matrix4x4f plt_matrix_identity();
Plt_Matrix4x4f plt_matrix_multiply(Plt_Matrix4x4f a, Plt_Matrix4x4f b);

Plt_Matrix4x4f plt_matrix_translate_make(Plt_Vector3f translate);
Plt_Matrix4x4f plt_matrix_scale_make(Plt_Vector3f scale);
Plt_Matrix4x4f plt_matrix_rotate_make(Plt_Vector3f rotate);

Plt_Matrix4x4f plt_matrix_perspective_make(float aspect_ratio, float fov, float near_z, float far_z);

Plt_Vector4f plt_matrix_multiply_vector4f(Plt_Matrix4x4f m, Plt_Vector4f v);

float plt_math_rad2deg(float rad);
float plt_math_deg2rad(float deg);

// MARK: Color

typedef struct Plt_Color8 {
	unsigned char b, g, r, a;
} Plt_Color8;

Plt_Color8 plt_color8_make(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

// MARK: Renderer

typedef struct Plt_Renderer Plt_Renderer;
typedef struct Plt_Mesh Plt_Mesh;
void plt_renderer_clear(Plt_Renderer *renderer, Plt_Color8 clear_color);
void plt_renderer_draw_mesh(Plt_Renderer *renderer, Plt_Mesh *mesh);
void plt_renderer_present(Plt_Renderer *renderer);

void plt_renderer_set_model_matrix(Plt_Renderer *renderer, Plt_Matrix4x4f matrix);
void plt_renderer_set_view_matrix(Plt_Renderer *renderer, Plt_Matrix4x4f matrix);
void plt_renderer_set_projection_matrix(Plt_Renderer *renderer, Plt_Matrix4x4f matrix);

Plt_Vector2i plt_renderer_get_framebuffer_size(Plt_Renderer *renderer);

// MARK: Application

typedef enum Plt_Application_Option {
	Plt_Application_Option_None = 0
} Plt_Application_Option;

typedef struct Plt_Application Plt_Application;
Plt_Application *plt_application_create(const char *title, unsigned int width, unsigned int height, Plt_Application_Option options);
void plt_application_destroy(Plt_Application **application);

bool plt_application_should_close(Plt_Application *application);
void plt_application_update(Plt_Application *application);

Plt_Renderer *plt_application_get_renderer(Plt_Application *application);

// MARK: Mesh

typedef struct Plt_Mesh Plt_Mesh;
Plt_Mesh *plt_mesh_create(int vertex_count);
void plt_mesh_destroy(Plt_Mesh **mesh);

void plt_mesh_set_position(Plt_Mesh *mesh, int index, Plt_Vector3f position);
Plt_Vector3f plt_mesh_get_position(Plt_Mesh *mesh, int index);

void plt_mesh_set_normal(Plt_Mesh *mesh, int index, Plt_Vector3f normal);
Plt_Vector3f plt_mesh_get_normal(Plt_Mesh *mesh, int index);

void plt_mesh_set_uv(Plt_Mesh *mesh, int index, Plt_Vector2f uv);
Plt_Vector2f plt_mesh_get_uv(Plt_Mesh *mesh, int index);

#endif
