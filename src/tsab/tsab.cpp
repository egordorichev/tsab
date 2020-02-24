#include <tsab/tsab.hpp>
#include <tsab/tsab_common.hpp>
#include <tsab/tsab_graphics.hpp>

#include <lit/lit.hpp>
#include <SDL2/SDL.h>

#include <cstring>

static LitState* state;
static SDL_Event event;

bool tsab_init() {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) != 0) {
		tsab_report_sdl_error();
		return false;
	}

	if (!tsab_graphics_init("tsab", 320, 180)) {
		SDL_Quit();
		return false;
	}

	state = lit_new_state();

	lit_init_api(state);
	lit_interpret_file(state, "main.lit");

	LitValue init = lit_get_global(state, CONST_STRING(state, "init"));

	if (IS_FUNCTION(init)) {
		lit_call(state, init, NULL, 0);
	}

	return true;
}

void tsab_quit() {
	lit_free_api(state);
	lit_free_state(state);

	tsab_graphics_quit();
	SDL_Quit();
}

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