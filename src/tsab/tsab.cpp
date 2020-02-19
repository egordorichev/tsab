#include <tsab/tsab.hpp>
#include <tsab/tsab_common.hpp>
#include <tsab/tsab_graphics.hpp>

#include <SDL2/SDL.h>

bool tsab_init() {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		tsab_report_sdl_error();
		return false;
	}

	if (!tsab_graphics_init("tsab", 640, 480)) {
		SDL_Quit();
		return false;
	}

	return true;
}

void tsab_quit() {
	tsab_graphics_quit();
	SDL_Quit();
}

SDL_Event event;

bool tsab_frame() {
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				return true;
		}
	}

	return false;
}
void tsab_loop() {
	while (true) {
		if (tsab_frame()) {
			return;
		}

		SDL_Delay(10);
	}
}