#pragma once

#include "platypus/platypus.h"

#include "SDL.h"

typedef struct Plt_Input_State {
	long long pressed_keys;
	unsigned int pressed_mouse_buttons;
	Plt_Vector2f mouse_movement;
} Plt_Input_State;

void plt_input_state_initialise(Plt_Input_State *state);
void plt_input_state_set_key_down(Plt_Input_State *state, Plt_Key key);
void plt_input_state_set_key_up(Plt_Input_State *state, Plt_Key key);
void plt_input_state_set_mouse_button_down(Plt_Input_State *state, Plt_Mouse_Button button);
void plt_input_state_set_mouse_button_up(Plt_Input_State *state, Plt_Mouse_Button button);

Plt_Key plt_key_from_sdl_keycode(SDL_Keycode keycode);
Plt_Mouse_Button plt_mouse_button_from_sdl_mouse_index(unsigned int index);
