#ifndef PLATYPUS_H
#define PLATYPUS_H

#include <stdbool.h>

// MARK: Math

#define plt_min(X, Y) (((X) < (Y)) ? (X) : (Y))
#define plt_max(X, Y) (((X) > (Y)) ? (X) : (Y))
#define plt_clamp(V, MIN, MAX) (plt_min(plt_max(V, MIN), MAX))

#define PLT_PI 3.1415926535897932384626f

typedef struct Plt_Rect {
	int x, y, width, height;
} Plt_Rect;

typedef struct Plt_Size {
	unsigned int width, height;
} Plt_Size;

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

typedef Plt_Vector4f Plt_Quaternion;

typedef struct Plt_Matrix4x4f {
	float columns[4][4];
} Plt_Matrix4x4f;

typedef struct Plt_Transform {
	Plt_Vector3f translation;
	Plt_Quaternion rotation;
	Plt_Vector3f scale;
} Plt_Transform;

typedef enum Plt_Shape_Type {
	Plt_Shape_Type_Sphere,
	Plt_Shape_Type_Box
} Plt_Shape_Type;

typedef struct Plt_Shape_Sphere {
	float radius;
} Plt_Shape_Sphere;

typedef struct Plt_Shape_Box {
	Plt_Vector3f size;
} Plt_Shape_Box;

Plt_Shape_Sphere plt_shape_sphere_make(float radius);
Plt_Shape_Box plt_shape_box_make(Plt_Vector3f size);

// Values defined in row-major format
Plt_Matrix4x4f plt_matrix_identity();
Plt_Matrix4x4f plt_matrix_zero();
Plt_Matrix4x4f plt_matrix_multiply(Plt_Matrix4x4f a, Plt_Matrix4x4f b);
Plt_Matrix4x4f plt_matrix_invert(Plt_Matrix4x4f m);

Plt_Matrix4x4f plt_matrix_translate_make(Plt_Vector3f translate);
Plt_Matrix4x4f plt_matrix_scale_make(Plt_Vector3f scale);
Plt_Matrix4x4f plt_matrix_rotate_make(Plt_Vector3f rotate);

Plt_Matrix4x4f plt_matrix_perspective_make(float aspect_ratio, float fov, float near_z, float far_z);

Plt_Transform plt_transform_create(Plt_Vector3f translation, Plt_Quaternion rotation, Plt_Vector3f scale);
Plt_Transform plt_transform_invert(Plt_Transform transform);
Plt_Matrix4x4f plt_transform_to_matrix(Plt_Transform transform);
Plt_Transform plt_transform_translate(Plt_Transform transform, Plt_Vector3f translation);
Plt_Transform plt_transform_rotate(Plt_Transform transform, Plt_Quaternion rotation);
Plt_Transform plt_transform_scale(Plt_Transform transform, Plt_Vector3f scale);

Plt_Vector4f plt_matrix_multiply_vector4f(Plt_Matrix4x4f m, Plt_Vector4f v);

Plt_Size plt_size_make(unsigned int width, unsigned int height);
Plt_Rect plt_rect_make(unsigned int x, unsigned int y, unsigned int width, unsigned int height);

Plt_Vector2i plt_vector2i_make(int x, int y);

Plt_Vector2f plt_vector2f_make(float x, float y);
Plt_Vector3f plt_vector3f_make(float x, float y, float z);
Plt_Vector4f plt_vector4f_make(float x, float y, float z, float w);

float plt_vector2f_dot_product(Plt_Vector2f a, Plt_Vector2f b);
float plt_vector3f_dot_product(Plt_Vector3f a, Plt_Vector3f b);

Plt_Vector3f plt_vector3f_cross(Plt_Vector3f a, Plt_Vector3f b);

Plt_Vector3f plt_vector3f_normalize(Plt_Vector3f v);

Plt_Vector2i plt_vector2i_add(Plt_Vector2i a, Plt_Vector2i b);
Plt_Vector2f plt_vector2f_add(Plt_Vector2f a, Plt_Vector2f b);
Plt_Vector3f plt_vector3f_add(Plt_Vector3f a, Plt_Vector3f b);
Plt_Vector4f plt_vector4f_add(Plt_Vector4f a, Plt_Vector4f b);

Plt_Vector2i plt_vector2i_subtract(Plt_Vector2i a, Plt_Vector2i b);
Plt_Vector3f plt_vector3f_subtract(Plt_Vector3f a, Plt_Vector3f b);

Plt_Vector2i plt_vector2i_multiply(Plt_Vector2i a, Plt_Vector2i b);
Plt_Vector2f plt_vector2f_multiply(Plt_Vector2f a, Plt_Vector2f b);
Plt_Vector3f plt_vector3f_multiply(Plt_Vector3f a, Plt_Vector3f b);
Plt_Vector4f plt_vector4f_multiply(Plt_Vector4f a, Plt_Vector4f b);

Plt_Vector2i plt_vector2i_multiply_scalar(Plt_Vector2i a, int b);
Plt_Vector2f plt_vector2f_multiply_scalar(Plt_Vector2f a, float b);
Plt_Vector3f plt_vector3f_multiply_scalar(Plt_Vector3f a, float b);
Plt_Vector4f plt_vector4f_multiply_scalar(Plt_Vector4f a, float b);

Plt_Vector2i plt_vector2i_divide_scalar(Plt_Vector2i a, int b);
Plt_Vector2f plt_vector2f_divide_scalar(Plt_Vector2f a, float b);
Plt_Vector3f plt_vector3f_divide_scalar(Plt_Vector3f a, float b);

Plt_Vector3f plt_vector3f_lerp(Plt_Vector3f a, Plt_Vector3f b, float i);

float plt_vector3f_distance(Plt_Vector3f a, Plt_Vector3f b);

float plt_math_rad2deg(float rad);
float plt_math_deg2rad(float deg);

Plt_Quaternion plt_quaternion_create_from_euler(Plt_Vector3f euler_angles);
Plt_Quaternion plt_quaternion_invert(Plt_Quaternion q);
Plt_Quaternion plt_quaternion_normalise(Plt_Quaternion q);
Plt_Quaternion plt_quaternion_add(Plt_Quaternion a, Plt_Quaternion b);
Plt_Quaternion plt_quaternion_multiply(Plt_Quaternion a, Plt_Quaternion b);
Plt_Vector3f plt_quaternion_to_euler(Plt_Quaternion q);
Plt_Vector3f plt_quaternion_rotate_vector(Plt_Quaternion q, Plt_Vector3f v);
Plt_Matrix4x4f plt_quaternion_to_matrix(Plt_Quaternion q);

// MARK: Color

typedef struct Plt_Color8 {
	unsigned char b, g, r, a;
} Plt_Color8;

Plt_Color8 plt_color8_make(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

Plt_Color8 plt_color8_add(Plt_Color8 a, Plt_Color8 b);
Plt_Color8 plt_color8_multiply_vector3f(Plt_Color8 a, Plt_Vector3f b);
Plt_Color8 plt_color8_multiply(Plt_Color8 a, Plt_Color8 b);
Plt_Color8 plt_color8_multiply_scalar(Plt_Color8 color, float s);
Plt_Color8 plt_color8_blend(Plt_Color8 a, Plt_Color8 b);

// MARK: ECS

typedef struct Plt_Input_State Plt_Input_State;
typedef struct Plt_Frame_State {
	// Time since last update (milliseconds)
	float delta_time;

	// Time since appliation was started (milliseconds)
	float application_time;

	Plt_Input_State *input_state;
} Plt_Frame_State;

#define PLT_WORLD_ENTITY_CAPACITY 8192
#define PLT_WORLD_COMPONENT_CAPACITY 64

typedef unsigned int Plt_Entity_ID;
typedef unsigned int Plt_Component_ID;

#define PLT_ENTITY_ID_NONE 0

typedef struct Plt_Entity {
	Plt_Entity_ID id;
	Plt_Entity_ID parent;
	unsigned long long components;
	const char *name;
	Plt_Transform transform;
} Plt_Entity;

typedef struct Plt_Component_Table_Entry {
	Plt_Entity_ID entity_id;
	char instance_data[];
} Plt_Component_Table_Entry;

typedef struct Plt_Component_Table {
	unsigned int entry_count;
	Plt_Component_Table_Entry *entries;
} Plt_Component_Table;

typedef struct Plt_World Plt_World;
typedef struct Plt_Renderer Plt_Renderer;
typedef struct Plt_Component {
	const char *name;

	Plt_Component_ID id;
	unsigned int data_size;

	Plt_Component_Table table;

	void (*init)(Plt_World *world, Plt_Entity_ID entity_id, void *instance_data);
	void (*update)(Plt_World *world, Plt_Entity_ID entity_id, void *instance_data, Plt_Frame_State state);
	void (*render)(Plt_World *world, Plt_Entity_ID entity_id, void *instance_data, Plt_Frame_State state, Plt_Renderer *renderer);
} Plt_Component;

typedef struct Plt_World {
	Plt_Entity_ID current_entity_id;
	Plt_Component_ID current_component_id;

	unsigned int entity_count;
	Plt_Entity entities[PLT_WORLD_ENTITY_CAPACITY];

	unsigned int deferred_destroy_count;
	Plt_Entity_ID deferred_destroy[PLT_WORLD_ENTITY_CAPACITY];

	unsigned int component_count;
	Plt_Component components[PLT_WORLD_COMPONENT_CAPACITY];

	// Runtime state
	bool is_updating;
} Plt_World;

Plt_World *plt_world_create();
void plt_world_destroy(Plt_World *world);

void plt_world_update(Plt_World *world, Plt_Frame_State state);
void plt_world_render(Plt_World *world, Plt_Frame_State state, Plt_Renderer *renderer);

Plt_Entity_ID plt_world_create_entity(Plt_World *world, const char *name, Plt_Entity_ID parent_id);
void plt_world_destroy_entity(Plt_World *world, Plt_Entity_ID entity_id);

void plt_world_entity_add_component(Plt_World *world, Plt_Entity_ID entity_id, const char *component_name);
void plt_world_entity_remove_component(Plt_World *world, Plt_Entity_ID entity_id, const char *component_name);

void *plt_world_get_component_instance_data(Plt_World *world, Plt_Entity_ID entity_id, const char *component_name);

Plt_Matrix4x4f plt_world_entity_get_model_matrix(Plt_World *world, Plt_Entity_ID entity_id);

Plt_Vector3f plt_entity_get_forward(Plt_World *world, Plt_Entity_ID entity_id);
Plt_Vector3f plt_entity_get_right(Plt_World *world, Plt_Entity_ID entity_id);
Plt_Vector3f plt_entity_get_up(Plt_World *world, Plt_Entity_ID entity_id);

Plt_Transform plt_world_entity_get_transform(Plt_World *world, Plt_Entity_ID entity_id);
void plt_world_entity_set_transform(Plt_World *world, Plt_Entity_ID entity_id, Plt_Transform transform);

const char *plt_world_entity_get_name(Plt_World *world, Plt_Entity_ID entity_id);
void plt_world_entity_set_name(Plt_World *world, Plt_Entity_ID entity_id, const char *name);

Plt_Entity_ID plt_world_entity_get_parent(Plt_World *world, Plt_Entity_ID entity_id);
void plt_world_entity_set_parent(Plt_World *world, Plt_Entity_ID entity_id, Plt_Entity_ID parent_id);

bool plt_world_get_entity_with_component(Plt_World *world, const char *component_name, Plt_Entity_ID *found);
void plt_world_get_entities_with_component(Plt_World *world, const char *component_name, Plt_Entity_ID *result_entities, unsigned int *result_entity_count);

void plt_world_register_component(Plt_World *world, const char *component_name, unsigned int data_size, void (*init)(Plt_World *world, Plt_Entity_ID entity_id, void *instance_data), void (*update)(Plt_World *world, Plt_Entity_ID entity_id, void *instance_data, Plt_Frame_State state), void (*render)(Plt_World *world, Plt_Entity_ID entity_id, void *instance_data, Plt_Frame_State state, Plt_Renderer *renderer));

// MARK: Base Components

// Mesh Renderer
#define PLT_COMPONENT_MESH_RENDERER "mesh_renderer"

typedef struct Plt_Mesh Plt_Mesh;
typedef struct Plt_Texture Plt_Texture;
typedef struct Plt_Object_Type_Mesh_Renderer_Data {
	Plt_Mesh *mesh;
	Plt_Texture *texture;
	Plt_Color8 color;
} Plt_Object_Type_Mesh_Renderer_Data;

void plt_component_mesh_renderer_set_mesh(Plt_World *world, Plt_Entity_ID entity_id, Plt_Mesh *mesh);
void plt_component_mesh_renderer_set_texture(Plt_World *world, Plt_Entity_ID entity_id, Plt_Texture *texture);

// Billboard Renderer
typedef struct Plt_Object_Type_Billboard_Renderer_Data {
	Plt_Vector2f size;
	Plt_Texture *texture;
} Plt_Object_Type_Billboard_Renderer_Data;

// Camera
#define PLT_COMPONENT_CAMERA "camera"

typedef struct Plt_Object_Type_Camera_Data {
	// Vertical FOV in radians
	float fov;

	// Near and far clipping planes
	float near_z;
	float far_z;
} Plt_Object_Type_Camera_Data;

Plt_Matrix4x4f plt_component_camera_get_view_matrix(Plt_World *world, Plt_Entity_ID entity_id);
Plt_Matrix4x4f plt_component_camera_get_projection_matrix(Plt_World *world, Plt_Entity_ID entity_id, Plt_Size viewport);

// Flying Camera Controller
#define PLT_COMPONENT_FLYING_CAMERA_CONTROLLER "flying_camera_controller"

typedef struct Plt_Object_Type_Flying_Camera_Controller_Data {
	float speed;

	float pitch;
	float yaw;
} Plt_Object_Type_Flying_Camera_Controller_Data;

// Collider
#define PLT_COMPONENT_COLLIDER "collider"
typedef struct Plt_Object_Type_Collider_Data {
	Plt_Shape_Type shape_type;
	union {
		Plt_Shape_Box box_shape;
		Plt_Shape_Sphere sphere_shape;
	};
} Plt_Object_Type_Collider_Data;

void plt_component_collider_set_box_shape(Plt_World *world, Plt_Entity_ID entity_id, Plt_Shape_Box box);
void plt_component_collider_set_sphere_shape(Plt_World *world, Plt_Entity_ID entity_id, Plt_Shape_Sphere sphere);

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

#define PLT_LIGHTING_SETUP_MAX_DIRECTIONAL_LIGHTS 32
typedef struct Plt_Lighting_Setup {
	Plt_Vector3f directional_light_directions[PLT_LIGHTING_SETUP_MAX_DIRECTIONAL_LIGHTS];
	Plt_Vector3f directional_light_amounts[PLT_LIGHTING_SETUP_MAX_DIRECTIONAL_LIGHTS];
	unsigned int directional_light_count;

	Plt_Vector3f ambient_lighting;
} Plt_Lighting_Setup;

typedef struct Plt_Renderer Plt_Renderer;
typedef struct Plt_Mesh Plt_Mesh;
typedef struct Plt_Texture Plt_Texture;
typedef struct Plt_Font Plt_Font;
void plt_renderer_clear(Plt_Renderer *renderer, Plt_Color8 clear_color);
void plt_renderer_present(Plt_Renderer *renderer);

void plt_renderer_direct_draw_pixel(Plt_Renderer *renderer, Plt_Vector2i position, unsigned int depth, Plt_Color8 color);
void plt_renderer_direct_draw_colored_rect(Plt_Renderer *renderer, Plt_Rect rect, unsigned int depth, Plt_Color8 color);
void plt_renderer_direct_draw_texture(Plt_Renderer *renderer, Plt_Rect rect, unsigned int depth, Plt_Texture *texture);
void plt_renderer_direct_draw_texture_with_offset(Plt_Renderer *renderer, Plt_Rect rect, Plt_Vector2i texture_offset, unsigned int depth, Plt_Texture *texture);
void plt_renderer_direct_draw_scaled_texture(Plt_Renderer *renderer, Plt_Rect rect, unsigned int depth, Plt_Texture *texture);
void plt_renderer_direct_draw_text(Plt_Renderer *renderer, Plt_Vector2i position, Plt_Font *font, const char *text);

void plt_renderer_draw_mesh(Plt_Renderer *renderer, Plt_Mesh *mesh);
void plt_renderer_draw_billboard(Plt_Renderer *renderer, Plt_Vector2f size);

void plt_renderer_set_primitive_type(Plt_Renderer *renderer, Plt_Primitive_Type primitive_type);
void plt_renderer_set_point_size(Plt_Renderer *renderer, unsigned int size);
void plt_renderer_set_lighting_model(Plt_Renderer *renderer, Plt_Lighting_Model model);
void plt_renderer_set_render_color(Plt_Renderer *renderer, Plt_Color8 color);
void plt_renderer_set_lighting_setup(Plt_Renderer* renderer, Plt_Lighting_Setup setup);
void plt_renderer_bind_texture(Plt_Renderer *renderer, Plt_Texture *texture);

void plt_renderer_set_model_matrix(Plt_Renderer *renderer, Plt_Matrix4x4f matrix);
void plt_renderer_set_view_matrix(Plt_Renderer *renderer, Plt_Matrix4x4f matrix);
void plt_renderer_set_projection_matrix(Plt_Renderer *renderer, Plt_Matrix4x4f matrix);

Plt_Size plt_renderer_get_framebuffer_size(Plt_Renderer *renderer);

// MARK: Application

typedef enum Plt_Application_Option {
	Plt_Application_Option_None = 0,
	Plt_Application_Option_Fullscreen = 1 << 0
} Plt_Application_Option;

typedef struct Plt_Application Plt_Application;
Plt_Application *plt_application_create(const char *title, unsigned int width, unsigned int height, unsigned int scale, Plt_Application_Option options);
void plt_application_destroy(Plt_Application **application);

bool plt_application_should_close(Plt_Application *application);
void plt_application_update(Plt_Application *application);

Plt_Renderer *plt_application_get_renderer(Plt_Application *application);

void plt_application_set_clear_color(Plt_Application *application, Plt_Color8 clear_color);

void plt_application_set_world(Plt_Application *application, Plt_World *world);
Plt_World *plt_application_get_world(Plt_Application *application);

void plt_application_set_target_fps(Plt_Application *application, unsigned int fps);
unsigned int plt_application_get_target_fps(Plt_Application *application);

float plt_application_get_milliseconds_since_creation(Plt_Application *application);

// MARK: Input

typedef struct Plt_Input_State Plt_Input_State;

typedef enum Plt_Key {
	Plt_Key_None = 0,
	Plt_Key_A = 1 << 0,
	Plt_Key_B = 1 << 1,
	Plt_Key_C = 1 << 2,
	Plt_Key_D = 1 << 3,
	Plt_Key_E = 1 << 4,
	Plt_Key_F = 1 << 5,
	Plt_Key_G = 1 << 6,
	Plt_Key_H = 1 << 7,
	Plt_Key_I = 1 << 8,
	Plt_Key_J = 1 << 9,
	Plt_Key_K = 1 << 10,
	Plt_Key_L = 1 << 11,
	Plt_Key_M = 1 << 12,
	Plt_Key_N = 1 << 13,
	Plt_Key_O = 1 << 14,
	Plt_Key_P = 1 << 15,
	Plt_Key_Q = 1 << 16,
	Plt_Key_R = 1 << 17,
	Plt_Key_S = 1 << 18,
	Plt_Key_T = 1 << 19,
	Plt_Key_U = 1 << 20,
	Plt_Key_V = 1 << 21,
	Plt_Key_W = 1 << 22,
	Plt_Key_X = 1 << 23,
	Plt_Key_Y = 1 << 24,
	Plt_Key_Z = 1 << 25,
	Plt_Key_Up = 1 << 26,
	Plt_Key_Down = 1 << 27,
	Plt_Key_Left = 1 << 28,
	Plt_Key_Right = 1 << 29,
	Plt_Key_Space = 1 << 30,
	Plt_Key_Enter = 1 << 31,
	Plt_Key_Escape = 1 << 32,
} Plt_Key;

typedef enum Plt_Mouse_Button {
	Plt_Mouse_Button_None = 0,
	Plt_Mouse_Button_Left = 1 << 0,
	Plt_Mouse_Button_Middle = 1 << 1,
	Plt_Mouse_Button_Right = 1 << 2
} Plt_Mouse_Button;

Plt_Key plt_input_state_get_pressed_keys(Plt_Input_State *state);
Plt_Mouse_Button plt_input_state_get_pressed_mouse_buttons(Plt_Input_State *state);
Plt_Vector2f plt_input_state_get_mouse_movement(Plt_Input_State *state);

bool plt_input_state_is_key_down(Plt_Input_State *state, Plt_Key key);
bool plt_input_state_is_mouse_button_down(Plt_Input_State *state, Plt_Mouse_Button button);

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
Plt_Texture *plt_texture_create(Plt_Size size);
void plt_texture_destroy(Plt_Texture **texture);

Plt_Texture *plt_texture_create_with_bytes_nocopy(Plt_Size size, void *bytes);
Plt_Texture *plt_texture_load(const char *path);

Plt_Color8 plt_texture_get_pixel(Plt_Texture *texture, Plt_Vector2i pos);
void plt_texture_set_pixel(Plt_Texture *texture, Plt_Vector2i pos, Plt_Color8 value);

Plt_Color8 plt_texture_sample(Plt_Texture *texture, Plt_Vector2f pos);

void plt_texture_clear(Plt_Texture *texture, Plt_Color8 value);

Plt_Size plt_texture_get_size(Plt_Texture *texture);
Plt_Vector2f plt_texture_get_texel_size(Plt_Texture *texture);
Plt_Color8 *plt_texture_get_pixels(Plt_Texture *texture);

// MARK: Font

typedef struct Plt_Font Plt_Font;
Plt_Font *plt_font_create(Plt_Size atlas_size);
void plt_font_destroy(Plt_Font **font);

Plt_Font *plt_font_load(const char *path);

Plt_Texture *plt_font_get_texture(Plt_Font *font);
Plt_Size plt_font_get_character_size(Plt_Font *font);
Plt_Rect plt_font_get_rect_for_character(Plt_Font *font, char c);
Plt_Size plt_font_get_size_of_string(Plt_Font *font, const char *string);

#endif
