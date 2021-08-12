#include "plt_font.h"

const int PLT_FONT_CHARACTERS_PER_ROW = 16;
const int PLT_FONT_CHARACTERS_PER_COLUMN = 16;

typedef struct Plt_Font {
	Plt_Size atlas_size;
	Plt_Texture *texture;
} Plt_Font;

Plt_Font *plt_font_create(Plt_Size atlas_size) {
	Plt_Font *font = malloc(sizeof(Plt_Font));

	font->texture = plt_texture_create(atlas_size);

	return font;
}

void plt_font_destroy(Plt_Font **font) {
	plt_texture_destroy(&(*font)->texture);
	free(*font);
	*font = NULL;
}

Plt_Font *plt_font_load(const char *path) {
	Plt_Font *font = malloc(sizeof(Plt_Font));
	
	Plt_Texture *texture = plt_texture_load(path);
	font->texture = texture;
	font->atlas_size = plt_texture_get_size(texture);

	return font;
}

Plt_Texture *plt_font_get_texture(Plt_Font *font) {
	return font->texture;
}

Plt_Size plt_font_get_character_size(Plt_Font *font) {
	return (Plt_Size) {
		.width = font->atlas_size.width / PLT_FONT_CHARACTERS_PER_ROW,
		.height = font->atlas_size.height / PLT_FONT_CHARACTERS_PER_COLUMN
	};
}

Plt_Rect plt_font_get_rect_for_character(Plt_Font *font, char c) {
	Plt_Size character_size = plt_font_get_character_size(font);
	unsigned int i = c;
	return (Plt_Rect) {
		.x = (i % PLT_FONT_CHARACTERS_PER_COLUMN) * character_size.width,
		.y = (i / PLT_FONT_CHARACTERS_PER_ROW) * character_size.height,
		.width = character_size.width,
		.height = character_size.height
	};
}
