#include "platypus/platypus.h"

#include <stdlib.h>
#include "SDL.h"

#include "platypus/base/macros.h"

typedef struct Plt_Application {
	SDL_Window *window;
	SDL_Surface *surface;

	Plt_Color8 *framebuffer;
	unsigned int framebuffer_width, framebuffer_height;

	bool should_quit;
} Plt_Application;

void plt_application_render(Plt_Application *application);

Plt_Application *plt_application_create(const char *title, unsigned int width, unsigned int height, Plt_Application_Option options) {
	Plt_Application *application = malloc(sizeof(Plt_Application));

	application->framebuffer = malloc(sizeof(Plt_Color8) * width * height);
	application->framebuffer_width = width;
	application->framebuffer_height = height;

	if (SDL_Init(0)) {
		plt_abort("Failed initialising SDL.\n");
	}
	application->window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_RESIZABLE);
	plt_assert(application->window, "SDL window creation failed.\n");

	application->should_quit = false;

	return application;
}
void plt_application_destroy(Plt_Application **application) {
	free(*application);
	*application = NULL;
}

bool plt_application_should_close(Plt_Application *application) {
	return application->should_quit;
}
void plt_application_update(Plt_Application *application) {
	SDL_Event event;
	while (!SDL_PollEvent(&event)){
		if (event.type == SDL_QUIT) {
			application->should_quit = true;
		}
	}

	plt_application_render(application);
}

void plt_application_render(Plt_Application *application) {
	SDL_Surface *surface = SDL_GetWindowSurface(application->window);

	SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 255, 0, 0));

	SDL_UpdateWindowSurface(application->window);
}