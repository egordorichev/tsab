#ifndef TSAB_GRAPHICS_HPP
#define TSAB_GRAPHICS_HPP

#include <tsab/tsab_common.hpp>

#include <SDL.h>
#include <SDL_gpu.h>

bool tsab_graphics_init(LitState* state, LitInstance* config);
void tsab_graphics_handle_event(SDL_Event* event);
void tsab_graphics_get_ready();
void tsab_graphics_quit();
GPU_Target* tsab_graphics_get_current_target();

void tsab_graphics_set_title(const char* title);
void tsab_graphics_clear_screen();
void tsab_graphics_begin_frame(float dt);
void tsab_graphics_finish_frame();
GPU_Image* tsab_graphics_get_image(int id);
void tsab_graphics_add_image(GPU_Image* image);

void tsab_graphics_bind_api(LitState* state);

#endif