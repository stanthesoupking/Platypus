#pragma once

#include "SDL.h"

SDL_Window *plt_application_get_sdl_window(Plt_Application *application);
void plt_application_present(Plt_Application *application);