#include <tsab/tsab_common.hpp>

#include <SDL.h>
#include <iostream>

void tsab_report_sdl_error() {
	std::cout << "SDL Error: " << SDL_GetError() << std::endl;
}