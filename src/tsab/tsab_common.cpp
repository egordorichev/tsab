#include <tsab/tsab_common.hpp>
#include <tsab/tsab.hpp>

#include <SDL.h>
#include <iostream>

void tsab_report_sdl_error() {
	tsab_fatal_error(SDL_GetError());
}