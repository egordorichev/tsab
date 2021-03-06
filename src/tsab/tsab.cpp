#include <tsab/tsab.hpp>
#include <tsab/tsab_common.hpp>
#include <tsab/graphics/tsab_graphics.hpp>
#include <tsab/tsab_shaders.hpp>
#include <tsab/tsab_audio.hpp>
#include <tsab/tsab_input.hpp>
#include <tsab/tsab_ui.hpp>
#include <tsab/physics/tsab_physics.hpp>

#include "lit.hpp"

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

#include <cstring>
#include <iostream>

static bool tsab_inited;
static LitState* state = nullptr;
static SDL_Event event;

static LitInstance* tsab = nullptr;
static LitString* update_string = nullptr;
static LitString* render_string = nullptr;

static int start_time;
static int end_time;
static float delta;
static float time_per_frame = 1000.0f / 60;
static float fps;
static LitValue last_error = NULL_VALUE;

static LitInstance* configure();

extern "C" const char prefix[];

#ifdef EMBED_BYTECODE
extern "C" const char bytecode[];
#endif

static void error_callback(LitState* state, const char* message) {
	fflush(stdout);
	fprintf(stderr, "%s%s%s", COLOR_RED, message, COLOR_RESET);
	fflush(stderr);

	last_error = OBJECT_CONST_STRING(state, message);
}

static LitInterpretResult call_tsab_method(LitString* name, LitValue* args, uint arg_count) {
	return lit_find_and_call_method(state, OBJECT_VALUE(tsab), name, args, arg_count);
}

bool tsab_handle_call(LitInterpretResult result) {
	if (result.type == INTERPRET_OK || result.type == INTERPRET_INVALID) {
		return false;
	}

	call_tsab_method(CONST_STRING(state, "handleError"), &last_error, 1);
	last_error = NULL_VALUE;

	return true;
}

void tsab_call_method(const char* name, LitValue* args, uint arg_count) {
	tsab_handle_call(call_tsab_method(CONST_STRING(state, name), args, arg_count));
}

bool tsab_init(bool debug) {
	tsab_inited = true;

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		tsab_report_sdl_error();
		return false;
	}

	state = lit_new_state();
	state->error_fn = error_callback;

	lit_open_libraries(state);
	lit_add_definition(state, debug ? "DEBUG" : "RELEASE");
	lit_set_optimization_level(debug ? OPTIMIZATION_LEVEL_DEBUG : OPTIMIZATION_LEVEL_RELEASE);

	lit_interpret(state, "prefix", (char*) prefix);
	LitValue value = lit_get_global(state, CONST_STRING(state, "tsab"));

	if (!IS_INSTANCE(value)) {
		tsab_fatal_error("tsab instance is missing");
		return false;
	}

	tsab = AS_INSTANCE(value);
	LitInstance* config = configure();

	if (!tsab_graphics_init(state, config)) {
		tsab_report_sdl_error();
		return false;
	}

	tsab_ui_init();
	tsab_audio_init();
	tsab_input_init();

	TTF_Init();
	IMG_Init(IMG_INIT_PNG);

	tsab_graphics_bind_api(state);
	tsab_shaders_bind_api(state);
	tsab_input_bind_api(state);
	tsab_audio_bind_api(state);
	tsab_physics_bind_api(state);
	tsab_ui_bind_api(state);

	LitInterpretResult result;

	#ifdef EMBED_BYTECODE
		LitModule* module = main_module = lit_get_module(state, "main");

		if (main_module == nullptr) {
			std::cout << "Main module is missing\n";
			return false;
		}

		result = lit_interpret_module(state, main_module);
	#else
		result = lit_interpret_file(state, "main.lit");
	#endif

	if (!tsab_handle_call(result)) {
		tsab_handle_call(call_tsab_method(CONST_STRING(state, "init"), nullptr, 0));
	}

	update_string = CONST_STRING(state, "update");
	render_string = CONST_STRING(state, "render");

	tsab_graphics_get_ready();
	return true;
}

void tsab_quit() {
	if (!tsab_inited) {
		return;
	}

	tsab_inited = false;

	if (state != nullptr) {
		call_tsab_method(CONST_STRING(state, "destroy"), nullptr, 0);
		lit_free_state(state);
	}

	tsab_physics_quit(state);
	tsab_input_quit();
	tsab_shaders_quit();
	tsab_ui_quit();
	tsab_graphics_quit();
	tsab_audio_quit();

	IMG_Quit();
	TTF_Quit();
	SDL_Quit();
}

bool tsab_frame() {
	start_time = SDL_GetTicks();

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT: {
				return true;
			}
		}

		tsab_input_handle_event(&event);
		tsab_graphics_handle_event(&event);
	}

	float realDelta = delta / 1000.0f;
	LitValue dt = NUMBER_VALUE(realDelta);
	LitInterpretResult interpret_result = call_tsab_method(update_string, &dt, 1);

	if (interpret_result.type == INTERPRET_OK && IS_BOOL(interpret_result.result) && AS_BOOL(interpret_result.result)) {
		return true;
	}

	tsab_handle_call(interpret_result);

	tsab_input_update();
	tsab_graphics_begin_frame(realDelta);

	tsab_handle_call(call_tsab_method(render_string, nullptr, 0));
	tsab_graphics_finish_frame();

	delta = (float) end_time - (float) start_time;

	if (delta < time_per_frame) {
		SDL_Delay(time_per_frame - delta);
		delta = time_per_frame;
	}

	fps = 1000.0f / delta;

	start_time = end_time;
	end_time = SDL_GetTicks();

	return false;
}

void tsab_setup_loop() {
	end_time = SDL_GetTicks();
}

void tsab_loop() {
	while (true) {
		if (tsab_frame()) {
			return;
		}
	}
}

static LitInstance* configure() {
	#ifdef EMBED_BYTECODE
		LitInterpretResult conf = lit_interpret(state, "main", bytecode);
	#else
		LitInterpretResult conf = lit_interpret_file(state, "config.lit");
	#endif

	if (conf.type != INTERPRET_OK) {
		return nullptr;
	}

	conf = call_tsab_method(CONST_STRING(state, "configure"), nullptr, 0);

	if (tsab_handle_call(conf)) {
		return nullptr;
	}

	LitValue config = conf.result;

	if (IS_INSTANCE(config)) {
		return AS_INSTANCE(config);
	}

	return nullptr;
}

float tsab_get_dt() {
	return delta / 1000.0f;
}

LitState* tsab_get_state() {
	return state;
}

void tsab_error(const char* message) {
	std::cerr << message << std::endl;
}

void tsab_fatal_error(const char* message) {
	tsab_error(message);
	tsab_quit();
}