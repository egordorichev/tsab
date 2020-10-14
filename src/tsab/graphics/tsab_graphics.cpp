#include <tsab/graphics/tsab_graphics.hpp>
#include <tsab/graphics/tsab_texture_region.hpp>
#include <tsab/graphics/tsab_animation.hpp>
#include <tsab/tsab_shaders.hpp>
#include <tsab/tsab_common.hpp>

#include <SDL_gpu.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

#include <vector>
#include <string>
#include <regex>
#include <iostream>

#define CURRENT_TARGET current_target == nullptr ? screen : current_target->target

static SDL_Color current_color = { 255, 255, 255, 255 };
static float bg_color[] = { 0, 0, 0, 1 };

static SDL_Window* window;
static SDL_Renderer* renderer;

static GPU_Target *screen;
static GPU_Image *current_target;

static std::vector<GPU_Image *> image_list;
static std::vector<TTF_Font *> fonts;
static TTF_Font *active_font;
static bool pushed = false;
static bool window_hidden = true;
static float total_time = 0;

bool tsab_graphics_init(LitState* state, LitInstance* config) {
	int width = 640;
	int height = 480;
	int min_width = -1;
	int min_height = -1;
	int max_width = -1;
	int max_height = -1;

	char* title = (char*) "tsab";
	bool bordered = true;

	if (config != NULL) {
		LitValue w = lit_get_field(state, &config->fields, "window");

		if (IS_INSTANCE(w)) {
			LitTable* window_map = &AS_INSTANCE(w)->fields;
			LitValue value;

			if (IS_NUMBER(value = lit_get_field(state, window_map, "width"))) {
				width = AS_NUMBER(value);
			}

			if (IS_NUMBER(value = lit_get_field(state, window_map, "height"))) {
				height = AS_NUMBER(value);
			}

			if (IS_NUMBER(value = lit_get_field(state, window_map, "min_width"))) {
				min_width = AS_NUMBER(value);
			}

			if (IS_NUMBER(value = lit_get_field(state, window_map, "min_height"))) {
				min_height = AS_NUMBER(value);
			}

			if (IS_NUMBER(value = lit_get_field(state, window_map, "max_width"))) {
				max_width = AS_NUMBER(value);
			}

			if (IS_NUMBER(value = lit_get_field(state, window_map, "max_height"))) {
				max_height = AS_NUMBER(value);
			}

			if (IS_STRING(value = lit_get_field(state, window_map, "title"))) {
				title = AS_CSTRING(value);
			}

			if (IS_BOOL(value = lit_get_field(state, window_map, "bordered"))) {
				bordered = AS_BOOL(value);
			}
		}
	}

	window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_RESIZABLE);

	if (window == nullptr) {
		return false;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (window == nullptr) {
		return false;
	}

	GPU_SetInitWindow(SDL_GetWindowID(window));

	if (min_width != -1 && min_height != -1) {
		SDL_SetWindowMinimumSize(window, min_width, min_height);
	}

	if (max_width != -1 && max_height != -1) {
		SDL_SetWindowMaximumSize(window, max_width, max_height);
	}

	if (!bordered) {
		SDL_SetWindowBordered(window, SDL_FALSE);
	}

	screen = GPU_Init(width, height, GPU_DEFAULT_INIT_FLAGS);

	if (screen == nullptr) {
		return false;
	}

	GPU_ClearRGBA(screen, bg_color[0], bg_color[1], bg_color[2], bg_color[3]);
	GPU_Flip(screen);

	return true;
}

void tsab_graphics_handle_event(SDL_Event* event) {
	if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_RESIZED) {
		int w;
		int h;

		SDL_GetWindowSize(window, &w, &h);
		GPU_SetWindowResolution(w, h);
	}
}

GPU_Target* tsab_graphics_get_current_target() {
	return CURRENT_TARGET;
}

void tsab_graphics_set_title(const char* title) {
	SDL_SetWindowTitle(window, title);
}

void tsab_graphics_get_ready() {
	SDL_ShowWindow(window);
	window_hidden = false;
}

void tsab_graphics_quit() {
	for (int i = 0; i < image_list.size(); i++) {
		GPU_FreeImage(image_list[i]);
	}

	GPU_Quit();

	if (renderer != nullptr) {
		SDL_DestroyRenderer(renderer);
	}

	if (window != nullptr) {
		SDL_DestroyWindow(window);
	}
}

void tsab_graphics_begin_frame(float dt) {
	total_time += dt;
	GPU_ClearRGBA(screen, bg_color[0], bg_color[1], bg_color[2], bg_color[3]);
}

void tsab_graphics_finish_frame() {
	GPU_Flip(screen);
}

void tsab_graphics_clear_screen() {
	GPU_ClearRGBA(tsab_graphics_get_current_target(), bg_color[0], bg_color[1], bg_color[2], bg_color[3]);
}

GPU_Image* tsab_graphics_get_image(int id) {
	if (id < 0 || id >= image_list.size()) {
		return nullptr;
	}

	return image_list[id];
}

int tsab_graphics_add_image(GPU_Image* image) {
	image_list.push_back(image);
	return image_list.size() - 1;
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

	GPU_ClearRGBA(CURRENT_TARGET, bg_color[0], bg_color[1], bg_color[2], bg_color[3]);
	return NULL_VALUE;
}

LIT_METHOD(tsab_graphics_new_canvas) {
	double w = LIT_CHECK_NUMBER(0);
	double h = LIT_CHECK_NUMBER(1);

	GPU_Image *image = GPU_CreateImage(w, h, GPU_FORMAT_RGBA);
	GPU_SetImageFilter(image, GPU_FILTER_NEAREST);
	GPU_LoadTarget(image);

	image_list.push_back(image);

	return NUMBER_VALUE(image_list.size() - 1);
}

LIT_METHOD(tsab_graphics_set_canvas) {
	int id = LIT_GET_NUMBER(0, -1);

	if (id > -1 && id < image_list.size()) {
		current_target = image_list[id];
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

	image_list.push_back(image);
	return NUMBER_VALUE(image_list.size() - 1);
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
			if (IS_ARRAY(args[0])) {
				LitArray* array = AS_ARRAY(args[0]);

				if (array->values.count < 3) {
					lit_runtime_error_exiting(vm, "Expected at least 3 values in the color array");
				}

				for (int i = 0; i < fmin(4, array->values.count); i++) {
					LitValue value = array->values.values[i];

					if (IS_NUMBER(value)) {
						bg_color[i] = AS_NUMBER(value);
					}
				}
			} else {
				float value = (float) LIT_CHECK_NUMBER(0);

				for (int i = 0; i < 3; i++) {
					bg_color[i] = value;
				}

				bg_color[3] = 255;
			}
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
			if (IS_ARRAY(args[0])) {
				LitArray* array = AS_ARRAY(args[0]);

				if (array->values.count < 3) {
					lit_runtime_error_exiting(vm, "Expected at least 3 values in the color array");
				}

				for (int i = 0; i < fmin(4, array->values.count); i++) {
					LitValue value = array->values.values[i];

					if (IS_NUMBER(value)) {
						bg_color[i] = AS_NUMBER(value);
					}
				}
			} else {
				Uint8 value = (Uint8) LIT_CHECK_NUMBER(0);

				current_color.r = value;
				current_color.g = value;
				current_color.b = value;
				current_color.a = 255;
			}
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
	LIT_ENSURE_MIN_ARGS(1)
	tsab_shaders_set_textured(true);

	auto *target = CURRENT_TARGET;
	GPU_Image *what = nullptr;

	float x = LIT_GET_NUMBER(1, 0);
	float y = LIT_GET_NUMBER(2, 0);
	float a = LIT_GET_NUMBER(3, 0);
	float ox = LIT_GET_NUMBER(4, 0);
	float oy = LIT_GET_NUMBER(5, 0);
	float sx = LIT_GET_NUMBER(6, 1);
	float sy = LIT_GET_NUMBER(7, 1);
	float src_x = LIT_GET_NUMBER(8, 0);
	float src_y = LIT_GET_NUMBER(9, 0);

	TextureRegion* region = nullptr;

	if (IS_NUMBER(args[0])) {
		what = tsab_graphics_get_image(AS_NUMBER(args[0]));
	} else if (IS_INSTANCE(args[0])) {
		region = LIT_EXTRACT_DATA_FROM(args[0], TextureRegion);
		what = region->texture;
	}

	if (what == nullptr) {
		return NULL_VALUE;
	}

	float src_w = LIT_GET_NUMBER(10, what->w);
	float src_h = LIT_GET_NUMBER(11, what->h);

	if (region != nullptr) {
		src_x = region->x;
		src_y = region->y;
		src_w = region->w;
		src_h = region->h;
	}

	GPU_Rect r = GPU_MakeRect(src_x, src_y, src_w, src_h);
	GPU_SetRGBA(what, current_color.r, current_color.g, current_color.b, current_color.a);
	GPU_BlitTransformX(what, &r, target, x, y, ox, oy, a, sx, sy);

	return NULL_VALUE;
}

LIT_METHOD(tsab_graphics_circle) {
	tsab_shaders_set_textured(false);

	double x = LIT_CHECK_NUMBER(0);
	double y = LIT_CHECK_NUMBER(1);
	double r = LIT_CHECK_NUMBER(2);
	bool filled = LIT_GET_BOOL(3, true);

	if (filled) {
		GPU_CircleFilled(CURRENT_TARGET, x, y, r, current_color);
	} else {
		GPU_Circle(CURRENT_TARGET, x, y, r, current_color);
	}

	return NULL_VALUE;
}

LIT_METHOD(tsab_graphics_rectangle) {
	tsab_shaders_set_textured(false);

	double x = LIT_CHECK_NUMBER(0);
	double y = LIT_CHECK_NUMBER(1);
	double w = LIT_CHECK_NUMBER(2);
	double h = LIT_CHECK_NUMBER(3);
	bool filled = LIT_GET_BOOL(4, true);

	if (filled) {
		GPU_RectangleFilled(CURRENT_TARGET, x, y, x + w, y + h, current_color);
	} else {
		GPU_Rectangle(CURRENT_TARGET, x, y, x + w, y + h, current_color);
	}

	return NULL_VALUE;
}

LIT_METHOD(tsab_graphics_ellipse) {
	tsab_shaders_set_textured(false);

	double x = LIT_CHECK_NUMBER(0);
	double y = LIT_CHECK_NUMBER(1);
	double w = LIT_CHECK_NUMBER(2);
	double h = LIT_CHECK_NUMBER(3);
	double d = LIT_GET_NUMBER(4, 0);
	bool filled = LIT_GET_BOOL(5, true);

	if (filled) {
		GPU_EllipseFilled(CURRENT_TARGET, x, y, w, h, d, current_color);
	} else {
		GPU_Ellipse(CURRENT_TARGET, x, y, w, h, d, current_color);
	}

	return NULL_VALUE;
}

LIT_METHOD(tsab_graphics_triangle) {
	tsab_shaders_set_textured(false);

	double x1 = LIT_CHECK_NUMBER(0);
	double y1 = LIT_CHECK_NUMBER(1);
	double x2 = LIT_CHECK_NUMBER(2);
	double y2 = LIT_CHECK_NUMBER(3);
	double x3 = LIT_CHECK_NUMBER(4);
	double y3 = LIT_CHECK_NUMBER(5);
	bool filled = LIT_GET_BOOL(6, true);

	if (filled) {
		GPU_TriFilled(CURRENT_TARGET, x1, y1, x2, y2, x3, y3, current_color);
	} else {
		GPU_Tri(CURRENT_TARGET, x1, y1, x2, y2, x3, y3, current_color);
	}

	return NULL_VALUE;
}

LIT_METHOD(tsab_graphics_point) {
	tsab_shaders_set_textured(false);

	double x = LIT_CHECK_NUMBER(1);
	double y = LIT_CHECK_NUMBER(2);

	GPU_Pixel(CURRENT_TARGET, x + 0.5, y + 0.5, current_color);

	return NULL_VALUE;
}

LIT_METHOD(tsab_graphics_line) {
	tsab_shaders_set_textured(false);

	double x1 = LIT_CHECK_NUMBER(0);
	double y1 = LIT_CHECK_NUMBER(1);
	double x2 = LIT_CHECK_NUMBER(2);
	double y2 = LIT_CHECK_NUMBER(3);

	GPU_Line(CURRENT_TARGET, x1 + 0.5, y1 + 0.5, x2 + 0.5, y2 + 0.5, current_color);

	return NULL_VALUE;
}

LIT_METHOD(tsab_graphics_new_font) {
	const char *name = LIT_CHECK_STRING(0);
	int size = LIT_GET_NUMBER(1, 12);

	TTF_Font *font = TTF_OpenFont(name, size);

	if (font == nullptr) {
		std::cerr << "Failed to load font " << name << ": " << TTF_GetError() << std::endl;
		return NULL_VALUE;
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

extern "C" const char default_font[];
extern "C" const size_t default_font_len;

static void load_font() {
	TTF_Font *font = TTF_OpenFontRW(SDL_RWFromConstMem((void*) default_font, default_font_len), 0, 12);

	if (font == nullptr) {
		std::cerr << "Failed to load default font: " << TTF_GetError() << std::endl;
	}

	active_font = font;
	fonts.push_back(font);
}

LIT_METHOD(tsab_graphics_print) {
	tsab_shaders_set_textured(true);

	if (active_font == nullptr) {
		load_font();
	}

	const char *text = LIT_CHECK_STRING(0);
	double x = LIT_GET_NUMBER(1, 0);
	double y = LIT_GET_NUMBER(2, 0);
	double r = LIT_GET_NUMBER(3, 0);
	double sx = LIT_GET_NUMBER(4, 1);
	double sy = LIT_GET_NUMBER(5, 1);

	SDL_Surface *surface = TTF_RenderUTF8_Blended(active_font, text, current_color);
	GPU_Image *image = GPU_CopyImageFromSurface(surface);

	GPU_SetImageFilter(image, GPU_FILTER_NEAREST);
	GPU_BlitTransformX(image, nullptr, CURRENT_TARGET, x + image->w / 2.0f, y + image->h / 2.0f,image->w / 2.0f, image->h / 2.0f, r, sx, sy);

	SDL_FreeSurface(surface);
	return NULL_VALUE;
}

LIT_METHOD(tsab_graphics_printf) {
	tsab_shaders_set_textured(true);

	if (active_font == nullptr) {
		load_font();
	}

	const char *text = LIT_CHECK_STRING(0);
	double x = LIT_GET_NUMBER(1, 0);
	double y = LIT_GET_NUMBER(2, 0);
	double length = LIT_GET_NUMBER(3, 256);
	double r = LIT_GET_NUMBER(4, 0);
	double sx = LIT_GET_NUMBER(5, 1);
	double sy = LIT_GET_NUMBER(6, 1);

	SDL_Surface *surface = TTF_RenderText_Blended_Wrapped(active_font, text, current_color, length);
	GPU_Image *image = GPU_CopyImageFromSurface(surface);

	GPU_SetImageFilter(image, GPU_FILTER_NEAREST);
	GPU_BlitTransformX(image, nullptr, CURRENT_TARGET, x + image->w / 2.0f, y + image->h / 2.0f,image->w / 2.0f, image->h / 2.0f, r, sx, sy);

	SDL_FreeSurface(surface);
	return NULL_VALUE;
}

LIT_METHOD(tsab_graphics_set_camera) {
	double x = LIT_GET_NUMBER(0, 0);
	double y = LIT_GET_NUMBER(1, 0);
	double s = LIT_GET_NUMBER(2, 1);

	GPU_MatrixMode(CURRENT_TARGET, GPU_MODEL);

	if (pushed) {
		GPU_PopMatrix();
	}

	if (arg_count == 0) {
		pushed = false;
		return NULL_VALUE;
	}

	if (current_target == nullptr) {
		y *= -1;
	}

	pushed = true;
	GPU_PushMatrix();
	GPU_Scale(s, s, 1.0f);
	GPU_Translate(x, y, 0.0f);

	return NULL_VALUE;
}

LIT_METHOD(tsab_graphics_set_clip) {
	if (arg_count == 0) {
		GPU_UnsetClip(CURRENT_TARGET);
		return NULL_VALUE;
	}

	double x = LIT_CHECK_NUMBER(0);
	double y = LIT_CHECK_NUMBER(1);
	double w = LIT_CHECK_NUMBER(2);
	double h = LIT_CHECK_NUMBER(3);

	GPU_SetClip(CURRENT_TARGET, x, y, w, h);
	return NULL_VALUE;
}

LIT_METHOD(tsab_graphics_set_shader) {
	if (arg_count == 0) {
		tsab_shaders_disable();
		return NULL_VALUE;
	}

	LitInstance* shader = LIT_CHECK_INSTANCE(0);
	LitValue id = lit_get_field(vm->state, &shader->fields, "id");

	if (IS_NUMBER(id)) {
		tsab_shaders_enable((int) AS_NUMBER(id));
	}

	return NULL_VALUE;
}

LIT_METHOD(tsab_window_title_get) {
	return OBJECT_VALUE(OBJECT_CONST_STRING(vm->state, SDL_GetWindowTitle(window)));
}

LIT_METHOD(tsab_window_title_set) {
	const char* title = LIT_CHECK_STRING(0);
	SDL_SetWindowTitle(window, title);

	return NULL_VALUE;
}

LIT_METHOD(tsab_window_hidden_get) {
	return BOOL_VALUE(window_hidden);
}

LIT_METHOD(tsab_window_hidden_set) {
	window_hidden = LIT_CHECK_BOOL(0);

	if (window_hidden) {
		SDL_HideWindow(window);
	} else {
		SDL_ShowWindow(window);
	}

	return NULL_VALUE;
}

LIT_METHOD(tsab_window_cursor_get) {
	return !SDL_ShowCursor(SDL_QUERY) ? TRUE_VALUE : FALSE_VALUE;
}

LIT_METHOD(tsab_window_cursor_set) {
	bool hide = LIT_CHECK_BOOL(0);
	SDL_ShowCursor(hide ? SDL_DISABLE : SDL_ENABLE);

	return NULL_VALUE;
}

LIT_METHOD(tsab_window_minimize) {
	SDL_MinimizeWindow(window);
	return NULL_VALUE;
}

LIT_METHOD(tsab_window_maximize) {
	SDL_MaximizeWindow(window);
	return NULL_VALUE;
}

LIT_METHOD(tsab_window_restore) {
	SDL_RestoreWindow(window);
	return NULL_VALUE;
}

LIT_METHOD(tsab_window_raise) {
	SDL_RaiseWindow(window);
	return NULL_VALUE;

}

LIT_METHOD(tsab_window_time_get) {
	return NUMBER_VALUE(total_time);
}

LIT_METHOD(tsab_window_width_get) {
	int x;
	int y;

	SDL_GetWindowSize(window, &x, &y);
	return NUMBER_VALUE(x);
}

LIT_METHOD(tsab_window_height_get) {
	int x;
	int y;

	SDL_GetWindowSize(window, &x, &y);
	return NUMBER_VALUE(y);
}

LIT_METHOD(tsab_window_set_size) {
	int width = LIT_CHECK_NUMBER(0);
	int height = LIT_CHECK_NUMBER(1);

	SDL_SetWindowSize(window, width, height);
	GPU_SetWindowResolution(width, height);

	return NULL_VALUE;
}

LIT_METHOD(tsab_window_set_fullscreen) {
	bool fullscreen = LIT_CHECK_BOOL(0);
	bool desktop = LIT_GET_BOOL(1, true);

	GPU_SetFullscreen(fullscreen, desktop);

	return NULL_VALUE;
}

LIT_METHOD(tsab_window_x_get) {
	int x;
	int y;

	SDL_GetWindowPosition(window, &x, &y);
	return NUMBER_VALUE(x);
}

LIT_METHOD(tsab_window_y_get) {
	int x;
	int y;

	SDL_GetWindowPosition(window, &x, &y);
	return NUMBER_VALUE(y);
}

LIT_METHOD(tsab_window_set_position) {
	int x = LIT_CHECK_NUMBER(0);
	int y = LIT_CHECK_NUMBER(1);

	SDL_SetWindowPosition(window, x, y);

	return NULL_VALUE;
}

void tsab_graphics_bind_api(LitState* state) {
	LIT_BEGIN_CLASS("Window")
		LIT_BIND_STATIC_FIELD("title", tsab_window_title_get, tsab_window_title_set)
		LIT_BIND_STATIC_FIELD("hidden", tsab_window_hidden_get, tsab_window_hidden_set)
		LIT_BIND_STATIC_FIELD("showCursor", tsab_window_cursor_get, tsab_window_cursor_set)
		LIT_BIND_STATIC_METHOD("minimize", tsab_window_minimize)
		LIT_BIND_STATIC_METHOD("maximize", tsab_window_maximize)
		LIT_BIND_STATIC_METHOD("restore", tsab_window_restore)
		LIT_BIND_STATIC_METHOD("raise", tsab_window_raise)

		LIT_BIND_STATIC_GETTER("time", tsab_window_time_get)
		LIT_BIND_STATIC_GETTER("width", tsab_window_width_get)
		LIT_BIND_STATIC_GETTER("height", tsab_window_height_get)
		LIT_BIND_STATIC_METHOD("setSize", tsab_window_set_size)
		LIT_BIND_STATIC_METHOD("setFullscreen", tsab_window_set_fullscreen)

		LIT_BIND_STATIC_GETTER("x", tsab_window_x_get)
		LIT_BIND_STATIC_GETTER("y", tsab_window_y_get)
		LIT_BIND_STATIC_METHOD("setPosition", tsab_window_set_position)
	LIT_END_CLASS()

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
		LIT_BIND_STATIC_METHOD("printf", tsab_graphics_printf)

		LIT_BIND_STATIC_METHOD("setCamera", tsab_graphics_set_camera)
		LIT_BIND_STATIC_METHOD("setClip", tsab_graphics_set_clip)

		LIT_BIND_STATIC_METHOD("setShader", tsab_graphics_set_shader)
	LIT_END_CLASS()

	tsab_texture_region_bind_api(state);
	tsab_animation_bind_api(state);
}

#undef CURRENT_TARGET