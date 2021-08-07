#include "platypus/platypus.h"

#include <stdlib.h>
#include "SDL.h"

#include "platypus/framebuffer/plt_framebuffer.h"
#include "platypus/renderer/plt_renderer.h"
#include "platypus/world/plt_world.h"

#include "platypus/base/platform.h"
#include "platypus/base/macros.h"

#if PLT_PLATFORM_WINDOWS
#include <time.h>
#elif PLT_PLATFORM_MACOS
#include <unistd.h>
#include <sys/time.h>
#elif PLT_PLATFORM_LINUX
#include <unistd.h>
#include <time.h>
#endif

typedef struct Plt_Application {
	SDL_Window *window;

	SDL_Surface *framebuffer_surface;
	Plt_Framebuffer framebuffer;

	Plt_Renderer *renderer;
	Plt_World *world;

	bool should_quit;
	unsigned int target_frame_ms;

	float millis_at_creation;

	unsigned int scale;
	Plt_Color8 clear_color;
} Plt_Application;

float plt_application_current_milliseconds();
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
	application->framebuffer = (Plt_Framebuffer){
		.pixels = NULL,
		.width = width,
		.height = height
	};
	application->renderer = plt_renderer_create(application, application->framebuffer);

	application->millis_at_creation = plt_application_current_milliseconds();

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
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			application->should_quit = true;
		}
	}

	plt_application_update_framebuffer(application);

	if (application->world) {
		plt_world_update(application->world);

		plt_renderer_clear(application->renderer, plt_color8_make(35,45,30,255));
		plt_world_render(application->world, application->renderer);
		plt_renderer_present(application->renderer);
	}
}

void plt_application_update_framebuffer(Plt_Application *application) {
	SDL_Surface *window_surface = SDL_GetWindowSurface(application->window);

	if (application->scale == 1) {
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
		}

		application->framebuffer = (Plt_Framebuffer) {
			.pixels = application->framebuffer_surface->pixels,
			.width = scaled_size.x,
			.height = scaled_size.y
		};
	}
	
	plt_renderer_update_framebuffer(application->renderer, application->framebuffer);
}

SDL_Window *plt_application_get_sdl_window(Plt_Application *application) {
	return application->window;
}

Plt_Renderer *plt_application_get_renderer(Plt_Application *application) {
	return application->renderer;
}

void plt_application_set_world(Plt_Application *application, Plt_World *world) {
	application->world = world;
}

Plt_World *plt_application_get_world(Plt_Application *application) {
	return application->world;
}

float plt_application_current_milliseconds() {
#if PLT_PLATFORM_WINDOWS
	clock_t c = clock();
	float elapsed = ((float)c) / CLOCKS_PER_SEC * 1000.0f;
	return elapsed;
#elif PLT_PLATFORM_MACOS 
	struct timespec curTime;
	clock_gettime(_CLOCK_REALTIME, &curTime);
	return curTime.tv_nsec / 1000000.0f;
#elif PLT_PLATFORM_LINUX
	struct timespec curTime;
	clock_gettime(CLOCK_REALTIME, &curTime);
	return curTime.tv_nsec / 1000000.0f;
#endif
}

float plt_application_get_milliseconds_since_creation(Plt_Application *application) {
	return plt_application_current_milliseconds() - application->millis_at_creation;
}

// TODO: Create macro-defined versions of this for different scales
void plt_application_fast_blit(Plt_Application *application) {
	Plt_Color8 *src_pixels = application->framebuffer.pixels;
	unsigned int src_width = application->framebuffer.width;
	unsigned int src_height = application->framebuffer.height;
	
	SDL_Surface *dest_surface = SDL_GetWindowSurface(application->window);
	Plt_Color8 *dest_pixels = dest_surface->pixels;
	unsigned int dest_width = dest_surface->w;
	unsigned int dest_height = dest_surface->h;
	
	unsigned int scale_factor = dest_width/src_width;
	
	for (unsigned int y = 0; y < src_height; ++y) {
		unsigned int dy = y * scale_factor;
		
		// Draw stretched row
		for (unsigned int x = 0; x < src_width; ++x) {
			unsigned int dx = x * scale_factor;
			for (unsigned int i = 0; i < scale_factor; ++i) {
				dest_pixels[dy * dest_width + dx + i] = src_pixels[y * src_width + x];
			}
		}
		
		// Repeat it
		for (unsigned int i = 0; i < scale_factor; ++i) {
			memcpy(dest_pixels + (dy + i) * dest_width, dest_pixels + dy * dest_width, sizeof(Plt_Color8) * dest_width);
		}
	}
}

void plt_application_present(Plt_Application *application) {
	if (application->scale != 1) {
		plt_application_fast_blit(application);
	}

	SDL_UpdateWindowSurface(application->window);
}
