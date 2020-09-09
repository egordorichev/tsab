#ifndef TSAB_GRAPHICS_HPP
#define TSAB_GRAPHICS_HPP

#include <tsab/tsab_common.hpp>
#include <SDL2/SDL.h>

bool tsab_graphics_init(LitState* state, LitMap* config);
void tsab_graphics_handle_event(SDL_Event* event);

bool tsab_graphics_set_title(const char* title);
void tsab_graphics_clear_screen();

void tsab_graphics_get_ready();
void tsab_graphics_quit();

void tsab_graphics_begin_frame(float dt);
void tsab_graphics_finish_frame();

void tsab_graphics_bind_api(LitState* state);

#endif