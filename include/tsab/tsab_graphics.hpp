#ifndef TSAB_GRAPHICS_HPP
#define TSAB_GRAPHICS_HPP

#include <tsab/tsab_common.hpp>

bool tsab_graphics_init(const char* title, uint w, uint h);
bool tsab_graphics_set_title(const char* title);
void tsab_graphics_clear_screen();

void tsab_graphics_get_ready();
void tsab_graphics_quit();

void tsab_graphics_begin_frame();
void tsab_graphics_finish_frame();

void tsab_graphics_bind_api(LitState* state);

#endif