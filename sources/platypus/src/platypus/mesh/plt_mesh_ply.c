#include "plt_mesh.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "platypus/base/macros.h"

#define _PLT_MESH_READ_LINE_BUFFER_SIZE 2048

typedef enum Plt_Mesh_Ply_Token_Type {
	Plt_Mesh_Ply_Token_Type_Invalid,
	Plt_Mesh_Ply_Token_Type_EOF,
	Plt_Mesh_Ply_Token_Type_Magic_Number,
	Plt_Mesh_Ply_Token_Type_Comment,
	Plt_Mesh_Ply_Token_Type_Define_Element,
	Plt_Mesh_Ply_Token_Type_Define_Property,
	Plt_Mesh_Ply_Token_Type_End_Header,
} Plt_Mesh_Ply_Token_Type;

typedef enum Plt_Mesh_Ply_Element {
	Plt_Mesh_Ply_Element_Invalid,
	Plt_Mesh_Ply_Element_Vertex,
	Plt_Mesh_Ply_Element_Face,
} Plt_Mesh_Ply_Element;

typedef enum Plt_Mesh_Ply_Property {
	Plt_Mesh_Ply_Property_Invalid,
	Plt_Mesh_Ply_Property_Position_X,
	Plt_Mesh_Ply_Property_Position_Y,
	Plt_Mesh_Ply_Property_Position_Z,
	Plt_Mesh_Ply_Property_Normal_X,
	Plt_Mesh_Ply_Property_Normal_Y,
	Plt_Mesh_Ply_Property_Normal_Z,
	Plt_Mesh_Ply_Property_UV_X,
	Plt_Mesh_Ply_Property_UV_Y,
	Plt_Mesh_Ply_Property_Vertex_Index,
} Plt_Mesh_Ply_Property;

typedef struct Plt_Mesh_Ply_Token {
	Plt_Mesh_Ply_Token_Type type;
	
	union {
		// Plt_Mesh_Ply_Token_Type_Element_Definition
		struct {
			Plt_Mesh_Ply_Element element;
			unsigned int element_count;
		};
		
		// Plt_Mesh_Ply_Token_Type_Define_Property
		struct {
			Plt_Mesh_Ply_Property property;
		};
	};
} Plt_Mesh_Ply_Token;

typedef struct Plt_Mesh_Ply_Layout {
	unsigned int vertex_count;
	
	// Vertex attribute positions
	unsigned char vertex_attrib_position_x;
	unsigned char vertex_attrib_position_y;
	unsigned char vertex_attrib_position_z;
	unsigned char vertex_attrib_normal_x;
	unsigned char vertex_attrib_normal_y;
	unsigned char vertex_attrib_normal_z;
	unsigned char vertex_attrib_uv_x;
	unsigned char vertex_attrib_uv_y;
	
	// Total attributes per vertex
	unsigned char vertex_attrib_count;
	
	unsigned int face_count;
	unsigned char face_attrib_vertex_index;
	unsigned char face_attrib_count;
} Plt_Mesh_Ply_Layout;

 Plt_Mesh_Ply_Token _ply_next_token(FILE *f) {
	char buffer[_PLT_MESH_READ_LINE_BUFFER_SIZE];
	char t_buffer0[256];
	char t_buffer1[256];
	while(fgets(buffer, _PLT_MESH_READ_LINE_BUFFER_SIZE, f)) {
		// Tokenise
		
		// Element
		if (strncmp(buffer, "element", 7) == 0) {
			Plt_Mesh_Ply_Element element = Plt_Mesh_Ply_Element_Invalid;
			unsigned int element_count;

			sscanf(buffer, "element %s %d", t_buffer0, &element_count);
			if (strncmp(t_buffer0, "vertex", 6) == 0) {
				element = Plt_Mesh_Ply_Element_Vertex;
			} else if (strncmp(t_buffer0, "face", 4) == 0) {
				element = Plt_Mesh_Ply_Element_Face;
			}
			
			return (Plt_Mesh_Ply_Token) {
				.type = Plt_Mesh_Ply_Token_Type_Define_Element,
				.element = element,
				.element_count = element_count
			};
		}
		
		// Property
		if (strncmp(buffer, "property", 8) == 0) {
			Plt_Mesh_Ply_Property property = Plt_Mesh_Ply_Property_Invalid;

			sscanf(buffer, "property %s %s", t_buffer0, t_buffer1);
			
			if (strcmp(t_buffer1, "x") == 0) {
				property = Plt_Mesh_Ply_Property_Position_X;
			} else if (strcmp(t_buffer1, "y") == 0) {
				property = Plt_Mesh_Ply_Property_Position_Y;
			} else if (strcmp(t_buffer1, "z") == 0) {
				property = Plt_Mesh_Ply_Property_Position_Z;
			} else if (strcmp(t_buffer1, "nx") == 0) {
				property = Plt_Mesh_Ply_Property_Normal_X;
			} else if (strcmp(t_buffer1, "ny") == 0) {
				property = Plt_Mesh_Ply_Property_Normal_Y;
			} else if (strcmp(t_buffer1, "nz") == 0) {
				property = Plt_Mesh_Ply_Property_Normal_Z;
			} else if (strcmp(t_buffer1, "s") == 0) {
				property = Plt_Mesh_Ply_Property_UV_X;
			} else if (strcmp(t_buffer1, "t") == 0) {
				property = Plt_Mesh_Ply_Property_UV_Y;
			} else if (strcmp(t_buffer1, "vertex_indices") == 0) {
				property = Plt_Mesh_Ply_Property_Vertex_Index;
			}
			
			return (Plt_Mesh_Ply_Token) {
				.type = Plt_Mesh_Ply_Token_Type_Define_Property,
				.property = property
			};
		}
		
		// End header
		if (strncmp(buffer, "end_header", 10) == 0) {
			return (Plt_Mesh_Ply_Token) {
				.type = Plt_Mesh_Ply_Token_Type_End_Header
			};
		}
		
		// Comment
		if (strncmp(buffer, "comment", 7) == 0) {
			return (Plt_Mesh_Ply_Token) {
				.type = Plt_Mesh_Ply_Token_Type_Comment
			};
		}
		
		// Magic number
		if (strncmp(buffer, "ply", 3) == 0) {
			return (Plt_Mesh_Ply_Token) {
				.type = Plt_Mesh_Ply_Token_Type_Magic_Number
			};
		}
	}
	return (Plt_Mesh_Ply_Token) {
		.type = Plt_Mesh_Ply_Token_Type_EOF
	};
}

void _ply_next_values(FILE *f, void *values, bool are_integer) {
	char line[512];
	unsigned int line_pos = 0;
	unsigned int values_pos = 0;
	
	char c;
	while (true) {
		c = fgetc(f);
		if (c == '\0' || c == ' ' || c == '\n') {
			if (are_integer) {
				// Parse int
				unsigned int value;
				line[line_pos] = '\0';
				sscanf(line, "%d", &value);
				((unsigned int *)values)[values_pos++] = value;
			} else {
				// Parse float
				float value;
				sscanf(line, "%f", &value);
				((float *)values)[values_pos++] = value;
			}

			
			line[0] = '\0';
			line_pos = 0;
			
			if (c == '\n') {
				return;
			}
		} else {
			line[line_pos++] = c;
		}
	}
}

Plt_Mesh *plt_mesh_load_ply(const char *path) {
	Plt_Mesh_Ply_Token token;
	Plt_Mesh_Ply_Layout layout = {
		.vertex_count = 0,
		.vertex_attrib_position_x = 0,
		.vertex_attrib_position_y = 0,
		.vertex_attrib_position_z = 0,
		.vertex_attrib_normal_x = 0,
		.vertex_attrib_normal_y = 0,
		.vertex_attrib_normal_z = 0,
		.vertex_attrib_uv_x = 0,
		.vertex_attrib_uv_y = 0,	
		.vertex_attrib_count = 0,
		.face_count = 0,
		.face_attrib_vertex_index = 0,
		.face_attrib_count = 0
	};

	FILE *f = fopen(path, "r");
	
	if(!f) {
		printf("Error: Failed loading mesh at path: '%s'.\n", path);
		abort();
	}
	
	// Check for 'magic number' line
	token = _ply_next_token(f);
	plt_assert(token.type == Plt_Mesh_Ply_Token_Type_Magic_Number, "PLY file was missing magic number");
	
	bool finished = false;
	unsigned int line = 1;
	token = _ply_next_token(f);
	while(!finished) {
		switch (token.type) {
			case Plt_Mesh_Ply_Token_Type_End_Header:
			case Plt_Mesh_Ply_Token_Type_EOF:
				finished = true;
				continue;
				break;
				
			case Plt_Mesh_Ply_Token_Type_Define_Element: {
				
				switch (token.element) {
					case Plt_Mesh_Ply_Element_Vertex: {
						layout.vertex_count = token.element_count;
						
						// Gather vertex elements
						token = _ply_next_token(f);
						unsigned char attrib_pos = 0;
						while (token.type == Plt_Mesh_Ply_Token_Type_Define_Property) {
							switch (token.property) {
								case Plt_Mesh_Ply_Property_Position_X:
									layout.vertex_attrib_position_x = attrib_pos;
									break;
									
								case Plt_Mesh_Ply_Property_Position_Y:
									layout.vertex_attrib_position_y = attrib_pos;
									break;

								case Plt_Mesh_Ply_Property_Position_Z:
									layout.vertex_attrib_position_z = attrib_pos;
									break;
									
								case Plt_Mesh_Ply_Property_Normal_X:
									layout.vertex_attrib_normal_x = attrib_pos;
									break;
									
								case Plt_Mesh_Ply_Property_Normal_Y:
									layout.vertex_attrib_normal_y = attrib_pos;
									break;
									
								case Plt_Mesh_Ply_Property_Normal_Z:
									layout.vertex_attrib_normal_z = attrib_pos;
									break;
									
								case Plt_Mesh_Ply_Property_UV_X:
									layout.vertex_attrib_uv_x = attrib_pos;
									break;
									
								case Plt_Mesh_Ply_Property_UV_Y:
									layout.vertex_attrib_uv_y = attrib_pos;
									break;
									
								default:
									// Unexpected property on vertex
									break;
							}
							
							token = _ply_next_token(f);
							attrib_pos++;
						}
						layout.vertex_attrib_count = attrib_pos;
						continue;
					} break;
						
					case Plt_Mesh_Ply_Element_Face: {
						layout.face_count = token.element_count;
						
						// Gather face elements
						token = _ply_next_token(f);
						unsigned char attrib_pos = 0;
						while (token.type == Plt_Mesh_Ply_Token_Type_Define_Property) {
							switch (token.property) {
								case Plt_Mesh_Ply_Property_Vertex_Index:
									layout.face_attrib_vertex_index = attrib_pos;
									break;
									
								default:
									// Unexpected property on vertex
									break;
							}
							
							token = _ply_next_token(f);
							attrib_pos++;
						}
						layout.face_attrib_count = attrib_pos;
						continue;
					} break;
						
					case Plt_Mesh_Ply_Element_Invalid:
						// Invalid element
						break;
				}
			
			} break;
				
			default:
				// Unexpected token
				break;
		}
		token = _ply_next_token(f);
		++line;
	}
	
	Plt_Vector3f *positions = malloc(sizeof(Plt_Vector3f) * layout.vertex_count);
	Plt_Vector3f *normals = malloc(sizeof(Plt_Vector3f) * layout.vertex_count);
	Plt_Vector2f *uvs = malloc(sizeof(Plt_Vector2f) * layout.vertex_count);
	
	const unsigned int max_indices_per_face = 6; // Square = 2 triangles / 6 indices
	unsigned int *indices = malloc(sizeof(unsigned int) * layout.face_count * max_indices_per_face);

	// Read in vertices
	for (unsigned int i = 0; i < layout.vertex_count; ++i) {
		float values[16];
		_ply_next_values(f, values, false);
		
		positions[i].x = values[layout.vertex_attrib_position_x];
		positions[i].y = values[layout.vertex_attrib_position_y];
		positions[i].z = values[layout.vertex_attrib_position_z];
		normals[i].x = values[layout.vertex_attrib_normal_x];
		normals[i].y = values[layout.vertex_attrib_normal_y];
		normals[i].z = values[layout.vertex_attrib_normal_z];
		uvs[i].x = values[layout.vertex_attrib_uv_x];
		uvs[i].y = 1.0f - values[layout.vertex_attrib_uv_y]; // UV.y must be flipped
	}
	
	// Read in indices
	unsigned int index_count = 0;
	for (unsigned int i = 0; i < layout.face_count; ++i) {
		unsigned int values[32];
		_ply_next_values(f, values, true);
		
		unsigned int point_count = values[0];
		if (point_count == 3) {
			// Parse triangle
			indices[index_count++] = values[3];
			indices[index_count++] = values[2];
			indices[index_count++] = values[1];
		} else if (point_count == 4) {
			// Parse square
			indices[index_count++] = values[3];
			indices[index_count++] = values[2];
			indices[index_count++] = values[1];
			
			indices[index_count++] = values[4];
			indices[index_count++] = values[3];
			indices[index_count++] = values[1];
		} else {
//			abort();
		}
	}

	Plt_Mesh *mesh = plt_mesh_create(index_count);

	for (unsigned int i = 0; i < index_count; ++i) {
		unsigned int index = indices[i];
		plt_mesh_set_position(mesh, i, positions[index]);
		plt_mesh_set_normal(mesh, i, normals[index]);
		plt_mesh_set_uv(mesh, i, uvs[index]);
	}

	free(positions);
	free(normals);
	free(uvs);
	free(indices);
		
	return mesh;
}
