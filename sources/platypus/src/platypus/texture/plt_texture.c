#include "platypus/platypus.h"

#include <stdlib.h>
#include "platypus/base/macros.h"

#define STB_IMAGE_IMPLEMENTATION
#include "platypus/vendor/stb_image.h"

typedef struct Plt_Texture {
	Plt_Color8 *data;

	unsigned int width;
	unsigned int height;

	unsigned int row_length;
} Plt_Texture;

Plt_Texture *plt_texture_create(unsigned int width, unsigned int height) {
	Plt_Texture *texture = malloc(sizeof(Plt_Texture));

	texture->width = width;
	texture->height = height;

	texture->row_length = width * sizeof(Plt_Color8);
	texture->data = malloc(texture->row_length * height);

	return texture;
}

void plt_texture_destroy(Plt_Texture **texture) {
	free((*texture)->data);
	free(*texture);
	*texture = NULL;
}

Plt_Texture *plt_texture_create_with_bytes_nocopy(unsigned int width, unsigned int height, void *bytes) {
	Plt_Texture *texture = malloc(sizeof(Plt_Texture));

	texture->width = width;
	texture->height = height;

	texture->row_length = width * sizeof(Plt_Color8);
	texture->data = bytes;

	return texture;
}

Plt_Texture *plt_texture_load(const char *path) {
	int width, height, channels;

	Plt_Color8 *pixels = (Plt_Color8 *)stbi_load(path, &width, &height, &channels, 4);
	plt_assert(pixels, "Failed loading texture from path.\n");

	if (!pixels) {
		return NULL;
	}
	
	unsigned int total_pixels = width * height;
	for (unsigned int i = 0; i < total_pixels; ++i) {
		Plt_Color8 p = pixels[i];
		pixels[i] = (Plt_Color8){p.r, p.g, p.b, p.a};
	}

	return plt_texture_create_with_bytes_nocopy(width, height, pixels);
}

inline Plt_Color8 plt_texture_get_pixel(Plt_Texture *texture, Plt_Vector2i pos) {
	pos.x = plt_clamp(pos.x, 0, texture->width);
	pos.y = plt_clamp(pos.y, 0, texture->height);

	return texture->data[pos.y * texture->width + pos.x];
}

inline void plt_texture_set_pixel(Plt_Texture *texture, Plt_Vector2i pos, Plt_Color8 value) {
	if (pos.x < 0 || pos.x >= texture->width || pos.y < 0 || pos.y >= texture->height) {
		return;
	}
	
	texture->data[pos.y * texture->width + pos.x] = value;
}

Plt_Color8 plt_texture_sample(Plt_Texture *texture, Plt_Vector2f pos) {
	Plt_Vector2i pixel_pos = { pos.x * texture->width, pos.y * texture->height };
	return plt_texture_get_pixel(texture, pixel_pos);
}

void plt_texture_clear(Plt_Texture *texture, Plt_Color8 value) {
#ifdef __APPLE__
	// Fast path
	memset_pattern4(texture->data, &value, sizeof(Plt_Color8) * texture->width * texture->height);
#else
	// Slow path
	Plt_Color8 *pixels = texture->data;
	for (unsigned int i = 0; i < texture->width * texture->height; ++i) {
		pixels[i] = value;
	}
#endif
}

Plt_Vector2i plt_texture_get_size(Plt_Texture *texture) {
	return (Plt_Vector2i) { texture->width, texture->height };
}
