#include <tsab/tsab.hpp>
#include <tsab/tsab_common.hpp>
#include <tsab/tsab_graphics.hpp>
#include <tsab/tsab_shaders.hpp>
#include <tsab/tsab_input.hpp>

#include <lit/lit.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include <cstring>
#include <string>

static LitState* state;
static SDL_Event event;

static LitString* update_string;
static LitString* render_string;

static Uint32 start_time;
static Uint32 end_time;
static float delta;
static float time_per_frame = 1000.0f / 60;
static float fps;

static void configure();

static int window_width = 640;
static int window_height = 480;
static char* title = (char*) "tsab";

bool tsab_init() {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) != 0) {
		tsab_report_sdl_error();
		return false;
	}

	state = lit_new_state();

	lit_open_libraries(state);

	configure();

	if (!tsab_graphics_init(title, window_width, window_height)) {
		SDL_Quit();
		return false;
	}

	tsab_input_init();

	TTF_Init();
	IMG_Init(IMG_INIT_PNG);

	tsab_graphics_bind_api(state);
	tsab_shaders_bind_api(state);
	tsab_input_bind_api(state);

	lit_interpret_file(state, "main.lit", false);
	lit_call_function(state, lit_get_global_function(state, CONST_STRING(state, "init")), NULL, 0);

	update_string = CONST_STRING(state, "update");
	render_string = CONST_STRING(state, "render");

	tsab_graphics_get_ready();

	return true;
}

void tsab_quit() {
	lit_call_function(state, lit_get_global_function(state, CONST_STRING(state, "destroy")), NULL, 0);
	lit_free_state(state);

	tsab_input_quit();
	tsab_shaders_quit();
	tsab_graphics_quit();

	IMG_Quit();
	TTF_Quit();
	SDL_Quit();
}

bool tsab_frame() {
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT: {
				return true;
			}
		}

		tsab_input_handle_event(&event);
	}

	LitValue dt = NUMBER_VALUE(delta / 1000.0);
	LitValue result = lit_call_function(state, lit_get_global_function(state, update_string), &dt, 1).result;

	if (IS_BOOL(result) && AS_BOOL(result)) {
		return true;
	}

	tsab_input_update();
	tsab_graphics_begin_frame();

	lit_call_function(state, lit_get_global_function(state, render_string), NULL, 0);
	tsab_graphics_finish_frame();

	return false;
}

void tsab_loop() {
	end_time = SDL_GetTicks();

	while (true) {
		start_time = SDL_GetTicks();

		if (tsab_frame()) {
			return;
		}

		delta = end_time - start_time;

		if (delta < time_per_frame) {
			SDL_Delay(time_per_frame - delta);
			delta = time_per_frame;
		}

		fps = 1000.0f / delta;

		// TMP: just for debug
		tsab_graphics_set_title(std::to_string((int) fps).c_str());
		// Don't kill me for string conversion methods, this is DEBUG ONLY

		start_time = end_time;
		end_time = SDL_GetTicks();
	}
}

static void configure() {
	LitValue config = lit_interpret_file(state, "config.lit", false).result;

	if (IS_MAP(config)) {
		LitMap* map = AS_MAP(config);
		LitValue window = lit_get_map_field(state, map, "window");

		if (IS_MAP(window)) {
			LitMap* window_map = AS_MAP(window);
			LitValue value = lit_get_map_field(state, window_map, "width");

			if (IS_NUMBER(value)) {
				window_width = AS_NUMBER(value);
			}

			value = lit_get_map_field(state, window_map, "height");

			if (IS_NUMBER(value)) {
				window_height = AS_NUMBER(value);
			}

			value = lit_get_map_field(state, window_map, "title");

			if (IS_STRING(value)) {
				title = AS_CSTRING(value);
			}
		}
	}
}