#include "platypus/platypus.h"

#include <stdlib.h>
#include "platypus/base/macros.h"

typedef struct Plt_Texture {
	void *data;

	unsigned int width;
	unsigned int height;
	unsigned int channels;

	unsigned int row_length;

	Plt_Texture_Format format;
} Plt_Texture;

unsigned int plt_texture_format_get_length(Plt_Texture_Format format) {
	switch (format) {
		case Plt_Texture_Format_Byte:
			return 1;

		case Plt_Texture_Format_Float:
			return 4;
	}
}

Plt_Texture *plt_texture_create(unsigned int width, unsigned int height, unsigned int channels, Plt_Texture_Format format) {
	Plt_Texture *texture = malloc(sizeof(Plt_Texture));

	texture->width = width;
	texture->height = height;
	texture->channels = channels;
	texture->format = format;

	texture->row_length = width * channels * plt_texture_format_get_length(format);
	texture->data = malloc(texture->row_length * height);

	return texture;
}

void plt_texture_destroy(Plt_Texture **texture) {
	free((*texture)->data);
	free(*texture);
	*texture = NULL;
}

Plt_Vector4f plt_texture_get_pixel(Plt_Texture *texture, Plt_Vector2i pos) {
	pos.x = plt_clamp(pos.x, 0, texture->width);
	pos.y = plt_clamp(pos.y, 0, texture->height);

	unsigned int pixel_length = plt_texture_format_get_length(texture->format) * texture->channels;
	void *pixel_data = texture->data + texture->row_length * pos.y + pos.x * pixel_length;

	Plt_Vector4f output = {};
	switch (texture->format) {
		case Plt_Texture_Format_Byte: {
			switch (texture->channels) {
				case 1:
					output.x = ((char *)pixel_data)[0];
					break;

				case 2:
					output.x = ((char *)pixel_data)[0];
					output.y = ((char *)pixel_data)[1];
					break;

				case 3:
					output.x = ((char *)pixel_data)[0];
					output.y = ((char *)pixel_data)[1];
					output.z = ((char *)pixel_data)[2];
					break;

				case 4:
					output.x = ((char *)pixel_data)[0];
					output.y = ((char *)pixel_data)[1];
					output.z = ((char *)pixel_data)[2];
					output.w = ((char *)pixel_data)[3];
					break;
			}
		} break;

		case Plt_Texture_Format_Float: {
			switch (texture->channels) {
				case 1:
					output.x = ((float *)pixel_data)[0];
					break;

				case 2:
					output.x = ((float *)pixel_data)[0];
					output.y = ((float *)pixel_data)[1];
					break;

				case 3:
					output.x = ((float *)pixel_data)[0];
					output.y = ((float *)pixel_data)[1];
					output.z = ((float *)pixel_data)[2];
					break;

				case 4:
					output.x = ((float *)pixel_data)[0];
					output.y = ((float *)pixel_data)[1];
					output.z = ((float *)pixel_data)[2];
					output.w = ((float *)pixel_data)[3];
					break;
			}
		} break;
	}

	return output;
}

void plt_texture_set_pixel(Plt_Texture *texture, Plt_Vector2i pos, Plt_Vector4f value) {
	if (pos.x < 0 || pos.x >= texture->width || pos.y < 0 || pos.y >= texture->height) {
		return;
	}

	unsigned int pixel_length = plt_texture_format_get_length(texture->format) * texture->channels;
	void *pixel_data = texture->data + texture->row_length * pos.y + pos.x * pixel_length;

	switch (texture->format) {
		case Plt_Texture_Format_Byte: {
			switch (texture->channels) {
				case 1:
					((char *)pixel_data)[0] = value.x;
					break;

				case 2:
					((char *)pixel_data)[0] = value.x;
					((char *)pixel_data)[1] = value.y;
					break;

				case 3:
					((char *)pixel_data)[0] = value.x;
					((char *)pixel_data)[1] = value.y;
					((char *)pixel_data)[2] = value.z;
					break;

				case 4:
					((char *)pixel_data)[0] = value.x;
					((char *)pixel_data)[1] = value.y;
					((char *)pixel_data)[2] = value.z;
					((char *)pixel_data)[3] = value.w;
					break;
			}
		} break;

		case Plt_Texture_Format_Float: {
			switch (texture->channels) {
				case 1:
					((float *)pixel_data)[0] = value.x;
					break;

				case 2:
					((float *)pixel_data)[0] = value.x;
					((float *)pixel_data)[1] = value.y;
					break;

				case 3:
					((float *)pixel_data)[0] = value.x;
					((float *)pixel_data)[1] = value.y;
					((float *)pixel_data)[2] = value.z;
					break;

				case 4:
					((float *)pixel_data)[0] = value.x;
					((float *)pixel_data)[1] = value.y;
					((float *)pixel_data)[2] = value.z;
					((float *)pixel_data)[3] = value.w;
					break;
			}
		} break;
	}
}

void plt_texture_clear(Plt_Texture *texture, Plt_Vector4f value) {
	char pixel_data[16];

	unsigned int pixel_length = plt_texture_format_get_length(texture->format) * texture->channels;

	switch (texture->format) {
		case Plt_Texture_Format_Byte: {
			switch (texture->channels) {
				case 1:
					((char *)pixel_data)[0] = value.x;
					break;
					
				case 2:
					((char *)pixel_data)[0] = value.x;
					((char *)pixel_data)[1] = value.y;
					break;
					
				case 3:
					((char *)pixel_data)[0] = value.x;
					((char *)pixel_data)[1] = value.y;
					((char *)pixel_data)[2] = value.z;
					break;
					
				case 4:
					((char *)pixel_data)[0] = value.x;
					((char *)pixel_data)[1] = value.y;
					((char *)pixel_data)[2] = value.z;
					((char *)pixel_data)[3] = value.w;
					break;
			}
		} break;
			
		case Plt_Texture_Format_Float: {
			switch (texture->channels) {
				case 1:
					((float *)pixel_data)[0] = value.x;
					break;
					
				case 2:
					((float *)pixel_data)[0] = value.x;
					((float *)pixel_data)[1] = value.y;
					break;
					
				case 3:
					((float *)pixel_data)[0] = value.x;
					((float *)pixel_data)[1] = value.y;
					((float *)pixel_data)[2] = value.z;
					break;
					
				case 4:
					((float *)pixel_data)[0] = value.x;
					((float *)pixel_data)[1] = value.y;
					((float *)pixel_data)[2] = value.z;
					((float *)pixel_data)[3] = value.w;
					break;
			}
		} break;
	}

	// Fast path using memset
	if (pixel_length == 1) {
		memset(texture->data, pixel_data[0], texture->width * texture->height);
		return;
	}

	#ifdef __APPLE__
	if (pixel_length == 4) {
		memset_pattern4(texture->data, pixel_data, 4 * texture->width * texture->height);
		return;
	} else if (pixel_length == 8) {
		memset_pattern8(texture->data, pixel_data, 8 * texture->width * texture->height);
		return;
	} else if (pixel_length == 16) {
		memset_pattern16(texture->data, pixel_data, 16 * texture->width * texture->height);
		return;
	}
	#endif

	// Slow path
	for (unsigned int i = 0; i < texture->width * texture->height; ++i) {
		for (unsigned int j = 0; j < pixel_length; ++j) {
			(((char *)texture->data)[i * pixel_length + j]) = pixel_data[j];
		}
	}
}

Plt_Vector2i plt_texture_get_size(Plt_Texture *texture) {
	return (Plt_Vector2i) { texture->width, texture->height };
}
