#include <tsab/tsab_graphics.hpp>
#include <tsab/tsab_common.hpp>

#include <SDL_GPU/SDL_gpu.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include <vector>
#include <string>
#include <regex>
#include <iostream>

static SDL_Color current_color = { 255, 255, 255, 255 };
static float bg_color[] = { 0, 0, 0, 1 };

static SDL_Window* window;
static SDL_Renderer* renderer;

static GPU_Target *screen;
static GPU_Image *current_target;

static std::vector<GPU_Image *> canvas_list;
static std::vector<TTF_Font *> fonts;
static TTF_Font *active_font;

bool tsab_graphics_init(const char* title, uint w, uint h) {
	window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_RESIZABLE);

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

void tsab_graphics_get_ready() {
	SDL_ShowWindow(window);
}

void tsab_graphics_quit() {
	for (int i = 0; i < canvas_list.size(); i++) {
		GPU_FreeImage(canvas_list[i]);
	}

	GPU_Quit();

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
}

void tsab_graphics_begin_frame() {
	GPU_ClearRGBA(screen, bg_color[0], bg_color[1], bg_color[2], bg_color[3]);
}

void tsab_graphics_finish_frame() {
	GPU_Flip(screen);
}

void tsab_graphics_clear_screen() {
	GPU_ClearRGBA(screen, bg_color[0], bg_color[1], bg_color[2], bg_color[3]);
}

/*
 * API
 */

LIT_METHOD(tsab_graphics_flip) {
	GPU_Flip(screen);
	return NULL_VALUE;
}

LIT_METHOD(tsab_graphics_clear) {
	if (arg_count == 3) {
		for (int i = 0; i < 3; i++) {
			bg_color[i] = (float) LIT_CHECK_NUMBER(i);
		}

		bg_color[3] = 255;
	} else if (arg_count == 1) {
		float value = (float) LIT_CHECK_NUMBER(0);

		for (int i = 0; i < 3; i++) {
			bg_color[i] = value;
		}

		bg_color[3] = 255;
	} else if (arg_count == 2) {
		float value = (float) LIT_CHECK_NUMBER(0);

		for (int i = 0; i < 3; i++) {
			bg_color[i] = value;
		}

		bg_color[3] = (float) LIT_CHECK_NUMBER(1);
	} else if (arg_count == 4) {
		for (int i = 0; i < 4; i++) {
			bg_color[i] = (float) LIT_CHECK_NUMBER(i);
		}
	}

	GPU_ClearRGBA(current_target == nullptr ? screen : current_target->target, bg_color[0], bg_color[1], bg_color[2], bg_color[3]);
	return NULL_VALUE;
}

LIT_METHOD(tsab_graphics_new_canvas) {
	double w = LIT_CHECK_NUMBER(0);
	double h = LIT_CHECK_NUMBER(1);

	GPU_Image *image = GPU_CreateImage(w, h, GPU_FORMAT_RGBA);
	GPU_SetImageFilter(image, GPU_FILTER_NEAREST);
	GPU_LoadTarget(image);

	canvas_list.push_back(image);

	return NUMBER_VALUE(canvas_list.size() - 1);
}

LIT_METHOD(tsab_graphics_set_canvas) {
	int id = LIT_GET_NUMBER(0, -1);

	if (id > -1 && id < canvas_list.size()) {
		current_target = canvas_list[id];
	} else {
		current_target = nullptr;
	}

	return NULL_VALUE;
}

LIT_METHOD(tsab_graphics_new_image) {
	const char *name = LIT_CHECK_STRING(0);

	SDL_Surface *texture = IMG_Load(name);

	if (texture == nullptr) {
		std::cerr << SDL_GetError() << std::endl;
		return NUMBER_VALUE(-1);
	}

	GPU_Image *image = GPU_CopyImageFromSurface(texture);
	GPU_SetImageFilter(image, GPU_FILTER_NEAREST);
	GPU_LoadTarget(image);

	canvas_list.push_back(image);

	return NUMBER_VALUE(canvas_list.size() - 1);
}

LIT_METHOD(tsab_graphics_set_clear_color) {
	if (arg_count > 0 && IS_STRING(args[0])) {
		std::string color = std::string(AS_CSTRING(args[0]));

		if (color.size() != 7) {
			return NULL_VALUE;
		}

		std::regex pattern("#?([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})");
		std::smatch match;

		if (std::regex_match(color, match, pattern)) {
			bg_color[0] = std::stoul(match[1].str(), nullptr, 16);
			bg_color[1] = std::stoul(match[2].str(), nullptr, 16);
			bg_color[2] = std::stoul(match[3].str(), nullptr, 16);
			bg_color[3] = 255;
		}
	} else {
		if (arg_count == 3) {
			for (int i = 0; i < 3; i++) {
				bg_color[i] = (float) LIT_CHECK_NUMBER(i);
			}

			bg_color[3] = 255;
		} else if (arg_count == 1) {
			float value = (float) LIT_CHECK_NUMBER(0);

			for (int i = 0; i < 3; i++) {
				bg_color[i] = value;
			}

			bg_color[3] = 255;
		} else if (arg_count == 2) {
			float value = (float) LIT_CHECK_NUMBER(0);

			for (int i = 0; i < 3; i++) {
				bg_color[i] = value;
			}

			bg_color[3] = (float) LIT_CHECK_NUMBER(1);
		} else if (arg_count >= 4) {
			for (int i = 0; i < 4; i++) {
				bg_color[i] = (float) LIT_CHECK_NUMBER(i);
			}
		}
	}

	return NULL_VALUE;
}

LIT_METHOD(tsab_graphics_set_color) {
	if (arg_count > 0 && IS_STRING(args[0])) {
		std::string color = std::string(AS_CSTRING(args[0]));

		if (color.size() != 7) {
			return NULL_VALUE;
		}

		std::regex pattern("#?([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})");
		std::smatch match;

		if (std::regex_match(color, match, pattern)) {
			current_color.r = std::stoul(match[1].str(), nullptr, 16);
			current_color.g = std::stoul(match[2].str(), nullptr, 16);
			current_color.b = std::stoul(match[3].str(), nullptr, 16);
			current_color.a = 255;
		}
	} else {
		if (arg_count == 3) {
			current_color.r = (Uint8) LIT_CHECK_NUMBER(0);
			current_color.g = (Uint8) LIT_CHECK_NUMBER(1);
			current_color.b = (Uint8) LIT_CHECK_NUMBER(2);
			current_color.a = 255;
		} else if (arg_count == 1) {
			Uint8 value = (Uint8) LIT_CHECK_NUMBER(0);

			current_color.r = value;
			current_color.g = value;
			current_color.b = value;
			current_color.a = 255;
		} else if (arg_count == 2) {
			Uint8 value = (Uint8) LIT_CHECK_NUMBER(0);

			current_color.r = value;
			current_color.g = value;
			current_color.b = value;
			current_color.a = (Uint8) LIT_CHECK_NUMBER(1);
		} else if (arg_count >= 4) {
			current_color.r = (Uint8) LIT_CHECK_NUMBER(0);
			current_color.g = (Uint8) LIT_CHECK_NUMBER(1);
			current_color.b = (Uint8) LIT_CHECK_NUMBER(2);
			current_color.a = (Uint8) LIT_CHECK_NUMBER(3);
		}
	}

	return NULL_VALUE;
}

LIT_METHOD(tsab_graphics_draw) {
	// if (tsab_shaders_get_active() > -1) {
	//	GPU_SetUniformf(GPU_GetUniformLocation(tsab_shaders_get_active_shader(), "textured"), 1);
	// }

	auto *target = current_target == nullptr ? screen : current_target->target;

	GPU_Image *what = nullptr;
	int id = LIT_CHECK_NUMBER(0);

	if (id > -1 && id < canvas_list.size()) {
		what = canvas_list[id];
	}

	if (what == nullptr) {
		return NULL_VALUE;
	}

	double x = LIT_GET_NUMBER(1, 0);
	double y = LIT_GET_NUMBER(2, 0);
	double a = LIT_GET_NUMBER(3, 0);
	double ox = LIT_GET_NUMBER(4, 0);
	double oy = LIT_GET_NUMBER(5, 0);
	double sx = LIT_GET_NUMBER(6, 1);
	double sy = LIT_GET_NUMBER(7, 1);
	double src_x = LIT_GET_NUMBER(8, 0);
	double src_y = LIT_GET_NUMBER(9, 0);
	double src_w = LIT_GET_NUMBER(10, what->w);
	double src_h = LIT_GET_NUMBER(11, what->h);

	GPU_Rect r = GPU_MakeRect(src_x, src_y, src_w, src_h);
	GPU_SetRGBA(what, current_color.r, current_color.g, current_color.b, current_color.a);
	GPU_BlitTransformX(what, &r, target, x, y, ox, oy, a, sx, sy);

	return NULL_VALUE;
}

LIT_METHOD(tsab_graphics_circle) {
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

	return NULL_VALUE;
}

LIT_METHOD(tsab_graphics_rectangle) {
	// if (tsab_shaders_get_active() > -1) {
	//	GPU_SetUniformf(GPU_GetUniformLocation(tsab_shaders_get_active_shader(), "textured"), 0);
	// }

	double x = LIT_CHECK_NUMBER(0);
	double y = LIT_CHECK_NUMBER(1);
	double w = LIT_CHECK_NUMBER(2);
	double h = LIT_CHECK_NUMBER(3);
	bool filled = LIT_GET_BOOL(4, true);

	if (filled) {
		GPU_RectangleFilled(current_target == nullptr ? screen : current_target->target, x, y, x + w, y + h, current_color);
	} else {
		GPU_Rectangle(current_target == nullptr ? screen : current_target->target, x, y, x + w, y + h, current_color);
	}

	return NULL_VALUE;
}

LIT_METHOD(tsab_graphics_ellipse) {
	// if (tsab_shaders_get_active() > -1) {
	//	GPU_SetUniformf(GPU_GetUniformLocation(tsab_shaders_get_active_shader(), "textured"), 0);
	// }

	double x = LIT_CHECK_NUMBER(0);
	double y = LIT_CHECK_NUMBER(1);
	double w = LIT_CHECK_NUMBER(2);
	double h = LIT_CHECK_NUMBER(3);
	double d = LIT_GET_NUMBER(4, 0);
	bool filled = LIT_GET_BOOL(5, true);

	if (filled) {
		GPU_EllipseFilled(current_target == nullptr ? screen : current_target->target, x, y, w, h, d, current_color);
	} else {
		GPU_Ellipse(current_target == nullptr ? screen : current_target->target, x, y, w, h, d, current_color);
	}

	return NULL_VALUE;
}

LIT_METHOD(tsab_graphics_triangle) {
	// if (tsab_shaders_get_active() > -1) {
	//	GPU_SetUniformf(GPU_GetUniformLocation(tsab_shaders_get_active_shader(), "textured"), 0);
	// }

	double x1 = LIT_CHECK_NUMBER(0);
	double y1 = LIT_CHECK_NUMBER(1);
	double x2 = LIT_CHECK_NUMBER(2);
	double y2 = LIT_CHECK_NUMBER(3);
	double x3 = LIT_CHECK_NUMBER(4);
	double y3 = LIT_CHECK_NUMBER(5);
	bool filled = LIT_GET_BOOL(6, true);

	if (filled) {
		GPU_TriFilled(current_target == nullptr ? screen : current_target->target, x1, y1, x2, y2, x3, y3, current_color);
	} else {
		GPU_Tri(current_target == nullptr ? screen : current_target->target, x1, y1, x2, y2, x3, y3, current_color);
	}

	return NULL_VALUE;
}

LIT_METHOD(tsab_graphics_point) {
	// if (tsab_shaders_get_active() > -1) {
	//	GPU_SetUniformf(GPU_GetUniformLocation(tsab_shaders_get_active_shader(), "textured"), 0);
	// }

	double x = LIT_CHECK_NUMBER(1);
	double y = LIT_CHECK_NUMBER(2);

	GPU_Pixel(current_target == nullptr ? screen : current_target->target, x + 0.5, y + 0.5, current_color);

	return NULL_VALUE;
}

LIT_METHOD(tsab_graphics_line) {
	// if (tsab_shaders_get_active() > -1) {
	//	GPU_SetUniformf(GPU_GetUniformLocation(tsab_shaders_get_active_shader(), "textured"), 0);
	// }

	double x1 = LIT_CHECK_NUMBER(0);
	double y1 = LIT_CHECK_NUMBER(1);
	double x2 = LIT_CHECK_NUMBER(2);
	double y2 = LIT_CHECK_NUMBER(3);

	GPU_Line(current_target == nullptr ? screen : current_target->target, x1 + 0.5, y1 + 0.5, x2 + 0.5, y2 + 0.5, current_color);

	return NULL_VALUE;
}

LIT_METHOD(tsab_graphics_new_font) {
	const char *name = LIT_CHECK_STRING(0);
	int size = LIT_GET_NUMBER(1, 14);

	TTF_Font *font = TTF_OpenFont(name, size);

	if (font == nullptr) {
		std::cerr << "Failed to load font " << name << ": " << TTF_GetError() << std::endl;
		return -1;
	}

	if (active_font == nullptr) {
		active_font = font;
	}

	fonts.push_back(font);

	return NUMBER_VALUE(fonts.size() - 1);
}

LIT_METHOD(tsab_graphics_set_font) {
	int id = LIT_CHECK_NUMBER(0);

	if (id > - 1 && id < fonts.size()) {
		active_font = fonts[id];
	}

	return NULL_VALUE;
}

LIT_METHOD(tsab_graphics_print) {
	// if (tsab_shaders_get_active() > -1) {
	//	GPU_SetUniformf(GPU_GetUniformLocation(tsab_shaders_get_active_shader(), "textured"), 1);
	// }

	const char *text = LIT_CHECK_STRING(0);
	double x = LIT_GET_NUMBER(1, 0);
	double y = LIT_GET_NUMBER(2, 0);
	double r = LIT_GET_NUMBER(3, 0);
	double sx = LIT_GET_NUMBER(4, 1);
	double sy = LIT_GET_NUMBER(5, 1);

	SDL_Surface *surface = TTF_RenderUTF8_Blended(active_font, text, current_color);
	GPU_Image *image = GPU_CopyImageFromSurface(surface);
	GPU_BlitTransformX(image, nullptr, current_target == nullptr ? screen : current_target->target, x + image->w/2, y + image->h/2, image->w/2, image->h/2, r, sx, sy);
	SDL_FreeSurface(surface);

	return NULL_VALUE;
}

void tsab_graphics_bind_api(LitState* state) {
	LIT_BEGIN_CLASS("Graphics")
		LIT_BIND_STATIC_METHOD("flip", tsab_graphics_flip)
		LIT_BIND_STATIC_METHOD("clear", tsab_graphics_clear)
		LIT_BIND_STATIC_METHOD("setColor", tsab_graphics_set_color)
		LIT_BIND_STATIC_METHOD("setClearColor", tsab_graphics_set_clear_color)
		LIT_BIND_STATIC_METHOD("newCanvas", tsab_graphics_new_canvas)
		LIT_BIND_STATIC_METHOD("setCanvas", tsab_graphics_set_canvas)
		LIT_BIND_STATIC_METHOD("newImage", tsab_graphics_new_image)

		LIT_BIND_STATIC_METHOD("draw", tsab_graphics_draw)
		LIT_BIND_STATIC_METHOD("circle", tsab_graphics_circle)
		LIT_BIND_STATIC_METHOD("rectangle", tsab_graphics_rectangle)
		LIT_BIND_STATIC_METHOD("ellipse", tsab_graphics_ellipse)
		LIT_BIND_STATIC_METHOD("triangle", tsab_graphics_triangle)
		LIT_BIND_STATIC_METHOD("point", tsab_graphics_point)
		LIT_BIND_STATIC_METHOD("line", tsab_graphics_line)

		LIT_BIND_STATIC_METHOD("newFont", tsab_graphics_new_font)
		LIT_BIND_STATIC_METHOD("setFont", tsab_graphics_set_font)
		LIT_BIND_STATIC_METHOD("print", tsab_graphics_print)
	LIT_END_CLASS()
}