#include "platypus/platypus.h"

#include <stdlib.h>
#include "platypus/base/plt_macros.h"
#include "platypus/base/plt_platform.h"

#define STB_IMAGE_IMPLEMENTATION
#include "platypus/vendor/stb_image.h"

typedef struct Plt_Texture {
	Plt_Color8 *data;

	Plt_Size size;

	unsigned int row_length;
} Plt_Texture;

Plt_Texture *plt_texture_create(Plt_Size size) {
	Plt_Texture *texture = malloc(sizeof(Plt_Texture));

	texture->size = size;

	texture->row_length = size.width * sizeof(Plt_Color8);
	texture->data = malloc(texture->row_length * size.height);

	return texture;
}

void plt_texture_destroy(Plt_Texture **texture) {
	free((*texture)->data);
	free(*texture);
	*texture = NULL;
}

Plt_Texture *plt_texture_create_with_bytes_nocopy(Plt_Size size, void *bytes) {
	Plt_Texture *texture = malloc(sizeof(Plt_Texture));

	texture->size = size;

	texture->row_length = size.width * sizeof(Plt_Color8);
	texture->data = bytes;

	return texture;
}

Plt_Texture *plt_texture_load(const char *path) {
	Plt_Size size;
	int channels;

	Plt_Color8 *pixels = (Plt_Color8 *)stbi_load(path, (int *)&size.width, (int *)&size.height, &channels, 4);
	plt_assert(pixels, "Failed loading texture from path.\n");

	if (!pixels) {
		return NULL;
	}
	
	unsigned int total_pixels = size.width * size.height;
	for (unsigned int i = 0; i < total_pixels; ++i) {
		Plt_Color8 p = pixels[i];
		pixels[i] = (Plt_Color8){p.r, p.g, p.b, p.a};
	}

	return plt_texture_create_with_bytes_nocopy(size, pixels);
}

inline Plt_Color8 plt_texture_get_pixel(Plt_Texture *texture, Plt_Vector2i pos) {
	pos.x = plt_clamp(pos.x, 0, texture->size.width);
	pos.y = plt_clamp(pos.y, 0, texture->size.height);

	return texture->data[pos.y * texture->size.width + pos.x];
}

inline void plt_texture_set_pixel(Plt_Texture *texture, Plt_Vector2i pos, Plt_Color8 value) {
	pos.x = pos.x % texture->size.width;
	pos.y = pos.y % texture->size.height;
	texture->data[pos.y * texture->size.width + pos.x] = value;
}

Plt_Color8 plt_texture_sample(Plt_Texture *texture, Plt_Vector2f pos) {
	Plt_Vector2i pixel_pos = { pos.x * texture->size.width, pos.y * texture->size.height };
	return plt_texture_get_pixel(texture, pixel_pos);
}

void plt_texture_clear(Plt_Texture *texture, Plt_Color8 value) {
#ifdef PLT_PLATFORM_MACOS
	// Fast path
	memset_pattern4(texture->data, &value, sizeof(Plt_Color8) * texture->size.width * texture->size.height);
#else
	// Slow path
	Plt_Color8 *pixels = texture->data;
	unsigned int total_pixels = texture->size.width * texture->size.height;
	for (unsigned int i = 0; i < total_pixels; ++i) {
		pixels[i] = value;
	}
#endif
}

Plt_Size plt_texture_get_size(Plt_Texture *texture) {
	return texture->size;
}
