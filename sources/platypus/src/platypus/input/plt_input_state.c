#include "plt_input_state.h"

#include "SDL.h"

Plt_Key plt_input_state_get_pressed_Keys(Plt_Input_State *state) {
	return state->pressed_keys;
}

void plt_input_state_initialise(Plt_Input_State *state) {
	state->pressed_keys = 0;
}

void plt_input_state_set_key_down(Plt_Input_State *state, Plt_Key key) {
	state->pressed_keys |= key;
}

void plt_input_state_set_key_up(Plt_Input_State *state, Plt_Key key) {
	state->pressed_keys &= ~((long long)key);
}

Plt_Key plt_key_from_sdl_keycode(SDL_Keycode keycode) {
	switch (keycode) {
		case SDLK_RETURN: return Plt_Key_Enter;
		case SDLK_ESCAPE: return Plt_Key_Escape;
		case SDLK_SPACE: return Plt_Key_Space;
		case SDLK_UP: return Plt_Key_Up;
		case SDLK_DOWN: return Plt_Key_Down;
		case SDLK_LEFT: return Plt_Key_Left;
		case SDLK_RIGHT: return Plt_Key_Right;
		case SDLK_a: return Plt_Key_A;
		case SDLK_b: return Plt_Key_B;
		case SDLK_c: return Plt_Key_C;
		case SDLK_d: return Plt_Key_D;
		case SDLK_e: return Plt_Key_E;
		case SDLK_f: return Plt_Key_F;
		case SDLK_g: return Plt_Key_G;
		case SDLK_h: return Plt_Key_H;
		case SDLK_i: return Plt_Key_I;
		case SDLK_j: return Plt_Key_J;
		case SDLK_k: return Plt_Key_K;
		case SDLK_l: return Plt_Key_L;
		case SDLK_m: return Plt_Key_M;
		case SDLK_n: return Plt_Key_N;
		case SDLK_o: return Plt_Key_O;
		case SDLK_p: return Plt_Key_P;
		case SDLK_q: return Plt_Key_Q;
		case SDLK_r: return Plt_Key_R;
		case SDLK_s: return Plt_Key_S;
		case SDLK_t: return Plt_Key_T;
		case SDLK_u: return Plt_Key_U;
		case SDLK_v: return Plt_Key_V;
		case SDLK_w: return Plt_Key_W;
		case SDLK_x: return Plt_Key_X;
		case SDLK_y: return Plt_Key_Y;
		case SDLK_z: return Plt_Key_Z;
		default: return Plt_Key_None;
	}
}

Plt_Vector2f plt_input_state_get_mouse_movement(Plt_Input_State *state) {
	return state->mouse_movement;
}