#include <tsab/graphics/tsab_animation.hpp>
#include <tsab/graphics/tsab_graphics.hpp>

#define CUTE_ASEPRITE_IMPLEMENTATION
#include "cute_aseprite.h"

#include <SDL_gpu.h>

typedef struct {
	GPU_Image* texture;
} Animation;


LIT_METHOD(aseprite_constructor) {
	const char* path = LIT_CHECK_STRING(0);
	ase_t* ase = cute_aseprite_load_from_file(path, nullptr);

	if (ase == nullptr) {
		lit_runtime_error(vm, "Failed to open aseprite file %s", path);
	}

	int w = ase->frame_count * ase->w;
	int h = ase->layer_count * ase->h;
	auto size = w * h * 4;

	Uint8 pixels[size];
	memset(pixels, 0, size);

	for (int f = 0; f < ase->frame_count; f++) {
		auto frame = ase->frames[f];

		if (frame.cel_count == 0) {
			continue;
		}

		auto cell = frame.cels[0];

		int cw = fmin(cell.w, ase->w);
		int ch = fmin(cell.h, ase->h);

		auto pixelData = (Uint8*) frame.pixels;

    for (int celY = 0; celY < ch; celY++) {
			for (int celX = 0; celX < cw; celX++) {
				auto index = (celX + celY * ase->w) * 4;
				auto to = (celX + f * cw + (celY) * w) * 4;

				for (int j = 0; j < 4; j++) {
					pixels[to + j] = pixelData[index + j];
				}
			}
		}
	}

	cute_aseprite_free(ase);
	SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(pixels, w, h, 32, 4 * w, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);

	GPU_Image* texture = GPU_CopyImageFromSurface(surface);
	GPU_SetImageFilter(texture, GPU_FILTER_NEAREST);
	GPU_SetSnapMode(texture, GPU_SNAP_NONE);

	tsab_graphics_add_image(texture);

	Animation* data = LIT_INSERT_DATA(Animation, nullptr);
	data->texture = texture;

	return instance;
}

LIT_METHOD(aseprite_update) {
	float dt = LIT_CHECK_NUMBER(0);

	return NULL_VALUE;
}

void tsab_animation_bind_api(LitState* state) {
	LIT_BEGIN_CLASS("Animation")
		LIT_BIND_CONSTRUCTOR(aseprite_constructor)
	LIT_END_CLASS()
}