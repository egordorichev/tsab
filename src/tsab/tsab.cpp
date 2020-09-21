#include <tsab/tsab.hpp>
#include <tsab/tsab_common.hpp>
#include <tsab/tsab_graphics.hpp>
#include <tsab/tsab_shaders.hpp>
#include <tsab/tsab_audio.hpp>
#include <tsab/tsab_input.hpp>
#include <tsab/tsab_ui.hpp>
#include <tsab/physics/tsab_physics.hpp>

#include <lit/lit.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include <cstring>
#include <iostream>

static LitState* state;
static SDL_Event event;

static LitInstance* tsab;
static LitString* update_string;
static LitString* render_string;

static int start_time;
static int end_time;
static float delta;
static float time_per_frame = 1000.0f / 60;
static float fps;

static LitInstance* configure();
static LitModule* main_module;

static void error_callback(LitState* state, LitErrorType type, const char* message, va_list args) {
	fflush(stdout);
	fprintf(stderr, COLOR_RED);
	vfprintf(stderr, message, args);
	fprintf(stderr, "%s\n", COLOR_RESET);
	fflush(stderr);
}

static LitInterpretResult call_tsab_method(LitString* name, LitValue* args, uint arg_count) {
	LitValue method;

	if (lit_table_get(&tsab->fields, name, &method)) {
		return lit_call(state, main_module, method, args, arg_count);
	}

	return (LitInterpretResult) { INTERPRET_INVALID, NULL_VALUE };
}

static bool handle(LitInterpretResult result) {
	if (result.type == INTERPRET_OK || result.type == INTERPRET_INVALID) {
		return false;
	}

	call_tsab_method(CONST_STRING(state, "handleError"), &result.result, 1);
	return true;
}

extern "C" const char prefix[];

#ifdef EMBED_BYTECODE
	extern "C" const char bytecode[];
#endif

bool tsab_init() {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		tsab_report_sdl_error();
		return false;
	}

	state = lit_new_state();
	state->error_fn = error_callback;

	lit_open_libraries(state);
	lit_interpret(state, "prefix", (char*) prefix);
	main_module = state->last_module;

	LitValue value = lit_get_global(state, CONST_STRING(state, "tsab"));

	if (!IS_INSTANCE(value)) {
		std::cout << "Failed to find tsab instance\n";
		return false;
	}

	tsab = AS_INSTANCE(value);
	LitInstance* config = configure();

	if (!tsab_graphics_init(state, config)) {
		SDL_Quit();
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
		main_module = lit_get_module(state, "main");

		if (main_module == NULL) {
			std::cout << "Main module is missing\n";
			return false;
		}

		result = lit_interpret_module(state, main_module);
	#else
		result = lit_interpret_file(state, "main.lit", false);
		main_module = state->last_module;
	#endif

	if (!handle(result)) {
		handle(call_tsab_method(CONST_STRING(state, "init"), NULL, 0));
	}

	update_string = CONST_STRING(state, "update");
	render_string = CONST_STRING(state, "render");

	tsab_graphics_get_ready();
	return true;
}

void tsab_quit() {
	lit_call_function(state, main_module, lit_get_global_function(state, CONST_STRING(state, "destroy")), NULL, 0);
	lit_free_state(state);

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

	float realDelta = delta / 1000.0;
	LitValue dt = NUMBER_VALUE(realDelta);
	LitInterpretResult interpret_result = call_tsab_method(update_string, &dt, 1);

	if (interpret_result.type == INTERPRET_OK && IS_BOOL(interpret_result.result) && AS_BOOL(interpret_result.result)) {
		return true;
	}

	handle(interpret_result);

	tsab_input_update();
	tsab_graphics_begin_frame(realDelta);

	handle(call_tsab_method(render_string, NULL, 0));
	tsab_graphics_finish_frame();

	delta = end_time - start_time;

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
		LitInterpretResult conf = lit_interpret_file(state, "config.lit", false);
	#endif

	if (conf.type != INTERPRET_OK) {
		return nullptr;
	}

	conf = call_tsab_method(CONST_STRING(state, "configure"), NULL, 0);

	if (handle(conf)) {
		return nullptr;
	}

	LitValue config = conf.result;

	if (IS_INSTANCE(config)) {
		return AS_INSTANCE(config);
	}

	return nullptr;
}

float tsab_get_dt() {
	return delta / 1000.0;
}