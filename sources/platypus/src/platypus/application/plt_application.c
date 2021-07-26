#include "platypus/platypus.h"

#include <stdlib.h>
#include "SDL.h"
#include "platypus/framebuffer/plt_framebuffer.h"
#include "platypus/renderer/plt_renderer.h"

#include "platypus/base/macros.h"

typedef struct Plt_Application {
	SDL_Window *window;

	SDL_Surface *framebuffer_surface;
	Plt_Framebuffer framebuffer;

	Plt_Renderer *renderer;

	bool should_quit;
	unsigned int target_frame_ms;

	unsigned int scale;
	Plt_Color8 clear_color;
} Plt_Application;

void plt_application_render(Plt_Application *application);
void plt_application_update_framebuffer(Plt_Application *application);

Plt_Application *plt_application_create(const char *title, unsigned int width, unsigned int height, unsigned int scale, Plt_Application_Option options) {
	Plt_Application *application = malloc(sizeof(Plt_Application));

	application->target_frame_ms = 16; // 60 FPS
	application->clear_color = plt_color8_make(80,80,80,255);
	application->scale = scale;

	if (SDL_Init(0)) {
		plt_abort("Failed initialising SDL.\n");
	}
	application->window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	plt_assert(application->window, "SDL window creation failed.\n");

	application->should_quit = false;
	application->renderer = plt_renderer_create(application, &application->framebuffer);

	application->framebuffer_surface = NULL;
	plt_application_update_framebuffer(application);

	return application;
}

void plt_application_destroy(Plt_Application **application) {
	plt_renderer_destroy(&(*application)->renderer);
	SDL_DestroyWindow((*application)->window);
	free(*application);
	*application = NULL;
}

bool plt_application_should_close(Plt_Application *application) {
	return application->should_quit;
}

void plt_application_update(Plt_Application *application) {
	// unsigned int frame_start = SDL_GetTicks();

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			application->should_quit = true;
		}
	}

	plt_application_update_framebuffer(application);

	// int wait_time = plt_max(application->target_frame_ms - (SDL_GetTicks() - frame_start), 0);
	// if (wait_time > 0) {
	// 	SDL_Delay(wait_time);
	// }
}

void plt_application_update_framebuffer(Plt_Application *application) {
	SDL_Surface *window_surface = SDL_GetWindowSurface(application->window);

	if (application->scale == 1) {
		if ((application->framebuffer.width != window_surface->w) || (application->framebuffer.height != window_surface->h)) {
			Plt_Texture *depth_texture = plt_texture_create(window_surface->w, window_surface->h, 1, Plt_Texture_Format_Float);
			plt_renderer_set_depth_texture(application->renderer, depth_texture);
		}

		application->framebuffer = (Plt_Framebuffer) {
			.pixels = window_surface->pixels,
			.width = window_surface->w,
			.height = window_surface->h
		};
	} else {
		Plt_Vector2i scaled_size = {window_surface->w / application->scale, window_surface->h / application->scale};
		
		if (application->framebuffer_surface && ((application->framebuffer_surface->w != scaled_size.x) || (application->framebuffer_surface->h != scaled_size.y))) {
			SDL_FreeSurface(application->framebuffer_surface);
			application->framebuffer_surface = NULL;
		}

		if (!application->framebuffer_surface) {
			application->framebuffer_surface = SDL_CreateRGBSurface(0, scaled_size.x, scaled_size.y, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

			Plt_Texture *depth_texture = plt_texture_create(scaled_size.x, scaled_size.y, 1, Plt_Texture_Format_Float);
			plt_renderer_set_depth_texture(application->renderer, depth_texture);
		}

		application->framebuffer = (Plt_Framebuffer) {
			.pixels = application->framebuffer_surface->pixels,
			.width = scaled_size.x,
			.height = scaled_size.y
		};
	}
}

SDL_Window *plt_application_get_sdl_window(Plt_Application *application) {
	return application->window;
}

Plt_Renderer *plt_application_get_renderer(Plt_Application *application) {
	return application->renderer;
}

void plt_application_present(Plt_Application *application) {
	if (application->scale != 1) {
		SDL_BlitScaled(application->framebuffer_surface, NULL, SDL_GetWindowSurface(application->window), NULL);
	}

	SDL_UpdateWindowSurface(application->window);
}
