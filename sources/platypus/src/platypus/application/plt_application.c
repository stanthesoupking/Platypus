#include "platypus/platypus.h"

#include <stdlib.h>
#include <math.h>
#include "SDL.h"

#include "platypus/framebuffer/plt_framebuffer.h"
#include "platypus/renderer/plt_renderer.h"
#include "platypus/world/plt_world.h"
#include "platypus/input/plt_input_state.h"

#include "platypus/base/platform.h"
#include "platypus/base/macros.h"

#if PLT_PLATFORM_WINDOWS
#include <time.h>
#include <windows.h>
#elif PLT_PLATFORM_UNIX
#include <unistd.h>
#include <time.h>
#endif

typedef struct Plt_Application {
	SDL_Window *window;

	SDL_Surface *framebuffer_surface;
	Plt_Framebuffer framebuffer;

	Plt_Renderer *renderer;
	Plt_World *world;
	Plt_Input_State input_state;

	bool should_quit;

	unsigned int target_fps;
	float target_frame_ms;

	double millis_at_creation;
	double millis_at_since_last_update;
	double millis_since_world_last_update;

	unsigned int scale;
	Plt_Color8 clear_color;
} Plt_Application;

double plt_application_current_milliseconds();
void plt_application_render(Plt_Application *application);
void plt_application_update_framebuffer(Plt_Application *application);

Plt_Application *plt_application_create(const char *title, unsigned int width, unsigned int height, unsigned int scale, Plt_Application_Option options) {
	Plt_Application *application = malloc(sizeof(Plt_Application));

	application->clear_color = plt_color8_make(80,80,80,255);
	application->scale = scale;

	if (SDL_Init(0)) {
		plt_abort("Failed initialising SDL.\n");
	}
	
	Uint32 flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
	if (options & Plt_Application_Option_Fullscreen) {
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}
	
	application->window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);
	plt_assert(application->window, "SDL window creation failed.\n");

	// SDL_CaptureMouse(SDL_TRUE);
	SDL_SetRelativeMouseMode(SDL_TRUE);

	application->should_quit = false;
	application->framebuffer = (Plt_Framebuffer){
		.pixels = NULL,
		.width = width,
		.height = height
	};
	application->renderer = plt_renderer_create(application, application->framebuffer);

	plt_input_state_initialise(&application->input_state);

	application->millis_at_creation = plt_application_current_milliseconds();
	application->millis_at_since_last_update = plt_application_current_milliseconds();
	application->millis_since_world_last_update = plt_application_current_milliseconds();
	plt_application_set_target_fps(application, 60);

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
	double current_millis = plt_application_current_milliseconds();
	float delta_time = current_millis - application->millis_since_world_last_update;
	application->millis_since_world_last_update = current_millis;
	
	Plt_Frame_State frame_state = {
		.delta_time = delta_time,
		.application_time = plt_application_get_milliseconds_since_creation(application),
		.input_state = &application->input_state
	};
	
	application->input_state.mouse_movement = (Plt_Vector2f){0, 0};
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			application->should_quit = true;
		} else if (event.type == SDL_KEYDOWN) {
			plt_input_state_set_key_down(&application->input_state, plt_key_from_sdl_keycode(event.key.keysym.sym));
		} else if (event.type == SDL_KEYUP) {
			plt_input_state_set_key_up(&application->input_state, plt_key_from_sdl_keycode(event.key.keysym.sym));
		} else if (event.type == SDL_MOUSEMOTION) {
			application->input_state.mouse_movement.x = event.motion.xrel;
			application->input_state.mouse_movement.y = event.motion.yrel;
		}
	}

	plt_application_update_framebuffer(application);

	if (application->world) {
		plt_world_update(application->world, frame_state);

		plt_renderer_clear(application->renderer, plt_color8_make(35,45,30,255));
		plt_world_render_scene(application->world, frame_state, application->renderer);
		plt_world_render_ui(application->world, frame_state, application->renderer);
		plt_renderer_present(application->renderer);
	}

	double time = plt_application_current_milliseconds();
	double frame_time = time - application->millis_at_since_last_update;
	double wait_time_ms = plt_max(application->target_frame_ms - frame_time, 0);
#if PLT_PLATFORM_WINDOWS
	Sleep(wait_time_ms);
#else
	usleep(wait_time_ms * 880);
#endif
	application->millis_at_since_last_update = plt_application_current_milliseconds();
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
		Plt_Vector2i scaled_size = {
			ceilf((float)window_surface->w / (float)application->scale),
			ceilf((float)window_surface->h / (float)application->scale)
		};
		
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

void plt_application_set_target_fps(Plt_Application *application, unsigned int fps) {
	application->target_fps = fps;
	application->target_frame_ms = 1000.0f/(float)fps;
}

unsigned int plt_application_get_target_fps(Plt_Application *application) {
	return application->target_fps;
}

double plt_application_current_milliseconds() {
#if PLT_PLATFORM_WINDOWS
	clock_t c = clock();
	double elapsed = ((double)c) / CLOCKS_PER_SEC * 1000.0;
	return elapsed;
#elif PLT_PLATFORM_UNIX
	struct timespec curTime;
	clock_gettime(CLOCK_MONOTONIC, &curTime);
	return (curTime.tv_sec * 1000.0) + (curTime.tv_nsec / 1000000.0);
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
		for (unsigned int i = 1; i < scale_factor; ++i) {
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
