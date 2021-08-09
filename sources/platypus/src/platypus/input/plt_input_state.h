#pragma once

#include "platypus/platypus.h"

typedef struct Plt_Input_State {
	long long pressed_keys;
} Plt_Input_State;

void plt_input_state_initialise(Plt_Input_State *state);
void plt_input_state_set_key_down(Plt_Input_State *state, Plt_Key key);
void plt_input_state_set_key_up(Plt_Input_State *state, Plt_Key key);

Plt_Key plt_key_from_sdl_keycode(SDL_Keycode keycode);