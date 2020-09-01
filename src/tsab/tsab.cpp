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

static LitState* state;
static SDL_Event event;

static LitString* update_string;
static LitString* render_string;

static Uint32 start_time;
static Uint32 end_time;
static float delta;
static float time_per_frame = 1000.0f / 60;
static float fps;

static LitMap* configure();
static LitModule* main_module;

static void error_callback(LitState* state, LitErrorType type, const char* message, va_list args) {
	fflush(stdout);
	fprintf(stderr, COLOR_RED);
	vfprintf(stderr, message, args);
	fprintf(stderr, "%s\n", COLOR_RESET);
	fflush(stderr);
}

bool tsab_init() {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) != 0) {
		tsab_report_sdl_error();
		return false;
	}

	state = lit_new_state();
	state->error_fn = error_callback;

	lit_open_libraries(state);
	LitMap* config = configure();

	if (!tsab_graphics_init(state, config)) {
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
	main_module = state->last_module;
	lit_call_function(state, main_module, lit_get_global_function(state, CONST_STRING(state, "init")), NULL, 0);

	update_string = CONST_STRING(state, "update");
	render_string = CONST_STRING(state, "render");

	tsab_graphics_get_ready();

	return true;
}

void tsab_quit() {
	lit_call_function(state, main_module, lit_get_global_function(state, CONST_STRING(state, "destroy")), NULL, 0);
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

	float realDelta = delta / 1000.0;
	LitValue dt = NUMBER_VALUE(realDelta);
	LitInterpretResult interpret_result = lit_call_function(state, main_module, lit_get_global_function(state, update_string), &dt, 1);

	if (interpret_result.type == INTERPRET_OK && IS_BOOL(interpret_result.result) && AS_BOOL(interpret_result.result)) {
		return true;
	}

	tsab_input_update();
	tsab_graphics_begin_frame(realDelta);

	lit_call_function(state, main_module, lit_get_global_function(state, render_string), NULL, 0);
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

		start_time = end_time;
		end_time = SDL_GetTicks();
	}
}

static LitMap* configure() {
	LitInterpretResult conf = lit_interpret_file(state, "config.lit", false);

	if (conf.type != INTERPRET_OK) {
		return nullptr;
	}

	LitValue config = conf.result;

	if (IS_MAP(config)) {
		return AS_MAP(config);
	}

	return nullptr;
}