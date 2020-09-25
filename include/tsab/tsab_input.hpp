#ifndef TSAB_INPUT_HPP
#define TSAB_INPUT_HPP

#include <SDL.h>
#include <lit/lit.hpp>

#define MOUSE_1 0
#define MOUSE_2 1
#define MOUSE_3 2
#define MOUSE_4 3
#define MOUSE_5 4
#define MOUSE_WHEEL_UP 5
#define MOUSE_WHEEL_DOWN 6
#define MOUSE_WHEEL_LEFT 7
#define MOUSE_WHEEL_RIGHT 8

void tsab_input_init();
void tsab_input_quit();
void tsab_input_bind_api(LitState* state);
void tsab_input_update();
void tsab_input_handle_event(SDL_Event* event);

#endif