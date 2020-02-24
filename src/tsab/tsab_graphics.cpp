#include <tsab/tsab_graphics.hpp>
#include <tsab/tsab_common.hpp>

#include <SDL_GPU/SDL_gpu.h>
#include <SDL2/SDL.h>

static SDL_Color current_color = { 255, 255, 255, 255 };
static float bg_color[] = { 0, 0, 0 };

static SDL_Window* window;
static SDL_Renderer* renderer;

static GPU_Target *screen;
static GPU_Image *current_target;

bool tsab_graphics_init(const char* title, uint w, uint h) {
	window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

	if (window == nullptr) {
		tsab_report_sdl_error();
		return false;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (window == nullptr) {
		tsab_report_sdl_error();
		SDL_DestroyWindow(window);

		return false;
	}

	GPU_SetInitWindow(SDL_GetWindowID(window));
	SDL_SetWindowMinimumSize(window, w, h);

	screen = GPU_Init(w, h, GPU_DEFAULT_INIT_FLAGS);

	if (screen == nullptr) {
		tsab_report_sdl_error();

		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);

		return false;
	}

	GPU_ClearRGB(screen, bg_color[0], bg_color[1], bg_color[2]);
	GPU_Flip(screen);

	return true;
}

void tsab_graphics_quit() {
	GPU_Quit();

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
}

void tsab_graphics_begin_frame() {
	GPU_ClearRGB(screen, bg_color[0], bg_color[1], bg_color[2]);
}

void tsab_graphics_finish_frame() {
	GPU_Flip(screen);
}

LIT_NATIVE(tsab_graphics_circle) {
	// if (tsab_shaders_get_active() > -1) {
	//  	GPU_SetUniformf(GPU_GetUniformLocation(tsab_shaders_get_active_shader(), "textured"), 0);
	// }

	double x = LIT_CHECK_NUMBER(0);
	double y = LIT_CHECK_NUMBER(1);
	double r = LIT_CHECK_NUMBER(2);
	bool filled = LIT_GET_BOOL(3, true);

	if (filled) {
		GPU_CircleFilled(current_target == nullptr ? screen : current_target->target, x, y, r, current_color);
	} else {
		GPU_Circle(current_target == nullptr ? screen : current_target->target, x, y, r, current_color);
	}

	return 0;
}

void tsab_graphics_bind_api(LitState* state) {
	lit_define_native(state, "tsabGraphicsCircle", tsab_graphics_circle_native);
}