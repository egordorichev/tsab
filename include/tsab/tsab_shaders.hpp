#ifndef TSAB_SHADERS_HPP
#define TSAB_SHADERS_HPP

#include <SDL_GPU/SDL_gpu.h>
#include <lit/lit.hpp>

void tsab_shaders_quit();
void tsab_shaders_bind_api(LitState* state);
void tsab_shaders_enable(int id);
void tsab_shaders_disable();
void tsab_shaders_set_textured(bool textured);

int tsab_shaders_get_active();
Uint32 tsab_shaders_get_active_shader();

#endif