#ifndef PLATYPUS_H
#define PLATYPUS_H

#include <stdbool.h>

// MARK: Color

typedef struct Plt_Vector3 {
	float x, y, z;
} Plt_Vector3;

typedef struct Plt_Vector2 {
	float x, y;
} Plt_Vector2;

typedef struct Plt_Color8 {
	unsigned char b, g, r, a;
} Plt_Color8;

Plt_Color8 plt_color8_make(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

// MARK: Application

typedef enum Plt_Application_Option {
	Plt_Application_Option_None = 0
} Plt_Application_Option;

typedef struct Plt_Application Plt_Application;
Plt_Application *plt_application_create(const char *title, unsigned int width, unsigned int height, Plt_Application_Option options);
void plt_application_destroy(Plt_Application **application);

bool plt_application_should_close(Plt_Application *application);
void plt_application_update(Plt_Application *application);

// MARK: Mesh

typedef struct Plt_Mesh Plt_Mesh;
Plt_Mesh *plt_mesh_create(int vertex_count);
void plt_mesh_destroy(Plt_Mesh **mesh);

void plt_mesh_set_position(Plt_Mesh *mesh, int index, Plt_Vector3 position);
Plt_Vector3 plt_mesh_get_position(Plt_Mesh *mesh, int index);

void plt_mesh_set_normal(Plt_Mesh *mesh, int index, Plt_Vector3 normal);
Plt_Vector3 plt_mesh_get_normal(Plt_Mesh *mesh, int index);

void plt_mesh_set_uv(Plt_Mesh *mesh, int index, Plt_Vector2 uv);
Plt_Vector2 plt_mesh_get_uv(Plt_Mesh *mesh, int index);

#endif