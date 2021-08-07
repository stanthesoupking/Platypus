#ifndef PLATYPUS_H
#define PLATYPUS_H

#include <stdbool.h>

// MARK: Math

#define plt_min(X, Y) (((X) < (Y)) ? (X) : (Y))
#define plt_max(X, Y) (((X) > (Y)) ? (X) : (Y))
#define plt_clamp(V, MIN, MAX) (plt_min(plt_max(V, MIN), MAX))

#define PLT_PI 3.1415926535897932384626f

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

typedef struct Plt_Transform {
	Plt_Vector3f translation;
	Plt_Vector3f rotation;
	Plt_Vector3f scale;
} Plt_Transform;

// Values defined in row-major format
Plt_Matrix4x4f plt_matrix_create(float v[4][4]);
Plt_Matrix4x4f plt_matrix_identity();
Plt_Matrix4x4f plt_matrix_zero();
Plt_Matrix4x4f plt_matrix_multiply(Plt_Matrix4x4f a, Plt_Matrix4x4f b);

Plt_Matrix4x4f plt_matrix_translate_make(Plt_Vector3f translate);
Plt_Matrix4x4f plt_matrix_scale_make(Plt_Vector3f scale);
Plt_Matrix4x4f plt_matrix_rotate_make(Plt_Vector3f rotate);

Plt_Matrix4x4f plt_matrix_perspective_make(float aspect_ratio, float fov, float near_z, float far_z);

Plt_Matrix4x4f plt_transform_to_matrix(Plt_Transform transform);

Plt_Vector4f plt_matrix_multiply_vector4f(Plt_Matrix4x4f m, Plt_Vector4f v);

float plt_vector2f_dot_product(Plt_Vector2f a, Plt_Vector2f b);
float plt_vector3f_dot_product(Plt_Vector3f a, Plt_Vector3f b);

Plt_Vector3f plt_vector3f_normalize(Plt_Vector3f v);

Plt_Vector2f plt_vector2f_add(Plt_Vector2f a, Plt_Vector2f b);
Plt_Vector3f plt_vector3f_add(Plt_Vector3f a, Plt_Vector3f b);
Plt_Vector4f plt_vector4f_add(Plt_Vector4f a, Plt_Vector4f b);

Plt_Vector3f plt_vector3f_subtract(Plt_Vector3f a, Plt_Vector3f b);

Plt_Vector2f plt_vector2f_multiply_scalar(Plt_Vector2f a, float b);
Plt_Vector3f plt_vector3f_multiply_scalar(Plt_Vector3f a, float b);
Plt_Vector4f plt_vector4f_multiply_scalar(Plt_Vector4f a, float b);

Plt_Vector2f plt_vector2f_divide_scalar(Plt_Vector2f a, float b);
Plt_Vector3f plt_vector3f_divide_scalar(Plt_Vector3f a, float b);

Plt_Vector3f plt_vector3f_lerp(Plt_Vector3f a, Plt_Vector3f b, float i);

float plt_math_rad2deg(float rad);
float plt_math_deg2rad(float deg);

// MARK: Color

typedef struct Plt_Color8 {
	unsigned char b, g, r, a;
} Plt_Color8;

Plt_Color8 plt_color8_make(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

Plt_Color8 plt_color8_add(Plt_Color8 a, Plt_Color8 b);
Plt_Color8 plt_color8_multiply(Plt_Color8 a, Plt_Color8 b);
Plt_Color8 plt_color8_multiply_scalar(Plt_Color8 color, float s);

// MARK: Type-Object System

typedef struct Plt_World Plt_World;

typedef struct Plt_Object Plt_Object;
typedef unsigned int Plt_Object_Type_ID;
typedef struct Plt_Object {
	Plt_World *world;
	Plt_Object_Type_ID type;

	const char *name;
	Plt_Transform transform;

	void *type_data;
} Plt_Object;

typedef struct Plt_Renderer Plt_Renderer;
typedef struct Plt_Object_Type_Descriptor {
	Plt_Object_Type_ID id;
	unsigned int data_size;

	void (*update)(Plt_Object *object, void *type_data);
	void (*render)(Plt_Object *object, void *type_data, Plt_Renderer *renderer);
} Plt_Object_Type_Descriptor;

Plt_World *plt_world_create(unsigned int object_storage_capacity, Plt_Object_Type_Descriptor *type_descriptors, unsigned int type_descriptor_count, bool include_base_types);
void plt_world_destroy(Plt_World **world);

Plt_Object *plt_world_create_object(Plt_World *world, Plt_Object *parent, Plt_Object_Type_ID type, const char *name);
void plt_world_destroy_object(Plt_World *world, Plt_Object **object);

Plt_Object *plt_object_get_parent(Plt_Object *object);
Plt_Matrix4x4f plt_object_get_model_matrix(Plt_Object *object); 

// MARK: Base Object Types

#define PLT_BASE_TYPE_ID_OFFSET 512
const static Plt_Object_Type_ID Plt_Object_Type_None = 0;
const static Plt_Object_Type_ID Plt_Object_Type_Mesh = PLT_BASE_TYPE_ID_OFFSET + 1;
const static Plt_Object_Type_ID Plt_Object_Type_Camera = PLT_BASE_TYPE_ID_OFFSET + 2;

typedef struct Plt_Mesh Plt_Mesh;
typedef struct Plt_Texture Plt_Texture;
typedef struct Plt_Object_Type_Mesh_Data {
	Plt_Mesh *mesh;
	Plt_Texture *texture;
	Plt_Color8 color;
} Plt_Object_Type_Mesh_Data;

// MARK: Renderer

typedef enum Plt_Primitive_Type {
	Plt_Primitive_Type_Point,
	Plt_Primitive_Type_Line,
	Plt_Primitive_Type_Triangle
} Plt_Primitive_Type;

typedef enum Plt_Lighting_Model {
	Plt_Lighting_Model_Unlit = 0,
	Plt_Lighting_Model_Vertex_Lit = 1,
} Plt_Lighting_Model;

typedef struct Plt_Renderer Plt_Renderer;
typedef struct Plt_Mesh Plt_Mesh;
typedef struct Plt_Texture Plt_Texture;
void plt_renderer_clear(Plt_Renderer *renderer, Plt_Color8 clear_color);
void plt_renderer_draw_mesh(Plt_Renderer *renderer, Plt_Mesh *mesh);
void plt_renderer_present(Plt_Renderer *renderer);

void plt_renderer_set_primitive_type(Plt_Renderer *renderer, Plt_Primitive_Type primitive_type);
void plt_renderer_set_point_size(Plt_Renderer *renderer, unsigned int size);
void plt_renderer_set_lighting_model(Plt_Renderer *renderer, Plt_Lighting_Model model);
void plt_renderer_set_render_color(Plt_Renderer *renderer, Plt_Color8 color);
void plt_renderer_bind_texture(Plt_Renderer *renderer, Plt_Texture *texture);

void plt_renderer_set_ambient_lighting_color(Plt_Renderer *renderer, Plt_Color8 color);
void plt_renderer_set_directional_lighting_color(Plt_Renderer *renderer, Plt_Color8 color);
void plt_renderer_set_directional_lighting_direction(Plt_Renderer *renderer, Plt_Vector3f direction);

void plt_renderer_set_model_matrix(Plt_Renderer *renderer, Plt_Matrix4x4f matrix);
void plt_renderer_set_view_matrix(Plt_Renderer *renderer, Plt_Matrix4x4f matrix);
void plt_renderer_set_projection_matrix(Plt_Renderer *renderer, Plt_Matrix4x4f matrix);

Plt_Vector2i plt_renderer_get_framebuffer_size(Plt_Renderer *renderer);

// MARK: Application

typedef enum Plt_Application_Option {
	Plt_Application_Option_None = 0
} Plt_Application_Option;

typedef struct Plt_Application Plt_Application;
Plt_Application *plt_application_create(const char *title, unsigned int width, unsigned int height, unsigned int scale, Plt_Application_Option options);
void plt_application_destroy(Plt_Application **application);

bool plt_application_should_close(Plt_Application *application);
void plt_application_update(Plt_Application *application);

Plt_Renderer *plt_application_get_renderer(Plt_Application *application);

void plt_application_set_world(Plt_Application *application, Plt_World *world);
Plt_World *plt_application_get_world(Plt_Application *application);

float plt_application_get_milliseconds_since_creation(Plt_Application *application);

// MARK: Mesh

typedef struct Plt_Mesh Plt_Mesh;
Plt_Mesh *plt_mesh_create(int vertex_count);
void plt_mesh_destroy(Plt_Mesh **mesh);

Plt_Mesh *plt_mesh_create_cube(Plt_Vector3f size);

Plt_Mesh *plt_mesh_load_ply(const char *path);

void plt_mesh_set_position(Plt_Mesh *mesh, int index, Plt_Vector3f position);
Plt_Vector3f plt_mesh_get_position(Plt_Mesh *mesh, int index);

void plt_mesh_set_normal(Plt_Mesh *mesh, int index, Plt_Vector3f normal);
Plt_Vector3f plt_mesh_get_normal(Plt_Mesh *mesh, int index);

void plt_mesh_set_uv(Plt_Mesh *mesh, int index, Plt_Vector2f uv);
Plt_Vector2f plt_mesh_get_uv(Plt_Mesh *mesh, int index);

// MARK: Texture

typedef struct Plt_Texture Plt_Texture;
Plt_Texture *plt_texture_create(unsigned int width, unsigned int height);
void plt_texture_destroy(Plt_Texture **texture);

Plt_Texture *plt_texture_create_with_bytes_nocopy(unsigned int width, unsigned int height, void *bytes);
Plt_Texture *plt_texture_load(const char *path);

Plt_Color8 plt_texture_get_pixel(Plt_Texture *texture, Plt_Vector2i pos);
void plt_texture_set_pixel(Plt_Texture *texture, Plt_Vector2i pos, Plt_Color8 value);

Plt_Color8 plt_texture_sample(Plt_Texture *texture, Plt_Vector2f pos);

void plt_texture_clear(Plt_Texture *texture, Plt_Color8 value);

Plt_Vector2i plt_texture_get_size(Plt_Texture *texture);

#endif
