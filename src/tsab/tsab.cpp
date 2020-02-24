#include <tsab/tsab.hpp>
#include <tsab/tsab_common.hpp>
#include <tsab/tsab_graphics.hpp>

#include <lit/lit.hpp>
#include <SDL2/SDL.h>

#include <cstring>

static LitState* state;
static SDL_Event event;

static LitFunction* update_callback;
static LitFunction* render_callback;

static Uint32 start_time;
static Uint32 end_time;
static Uint32 fps;
static Uint32 time_per_frame = 1000 / 60;
static Uint32 delta;

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
	lit_open_libraries(state);

	tsab_graphics_bind_api(state);

	lit_interpret_file(state, "main.lit");
	lit_call_function(state, lit_get_global_function(state, CONST_STRING(state, "init")), NULL, 0);

	update_callback = lit_get_global_function(state, CONST_STRING(state, "update"));
	render_callback = lit_get_global_function(state, CONST_STRING(state, "render"));

	return true;
}

void tsab_quit() {
	lit_call_function(state, lit_get_global_function(state, CONST_STRING(state, "destroy")), NULL, 0);

	lit_free_api(state);
	lit_free_state(state);

	tsab_graphics_quit();
	SDL_Quit();
}

bool tsab_frame() {
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT: {
				return true;
			}
		}
	}

	if (update_callback != NULL) {
		LitValue dt = NUMBER_VALUE(delta / 1000.0);
		lit_call(state, OBJECT_VALUE(update_callback), &dt, 1);
	}

	tsab_graphics_begin_frame();

	if (render_callback != NULL) {
		lit_call(state, OBJECT_VALUE(render_callback), NULL, 0);
	}

	tsab_graphics_finish_frame();
	return false;
}

void tsab_loop() {
	while (true) {
		start_time = SDL_GetTicks();

		if (tsab_frame()) {
			return;
		}

		delta = end_time - start_time;

		if (delta < time_per_frame) {
			SDL_Delay(time_per_frame - delta);
			fps = 1000 / time_per_frame;
			delta = time_per_frame;
		} else {
			fps = 1000 / delta;
		}

		start_time = end_time;
		end_time = SDL_GetTicks();
	}
}