#include <tsab/graphics/tsab_animation.hpp>
#include <tsab/graphics/tsab_graphics.hpp>
#include <tsab/graphics/tsab_texture_region.hpp>

#define CUTE_ASEPRITE_IMPLEMENTATION
#include "cute_aseprite.h"

#include <SDL_gpu.h>

#include <vector>
#include <map>

typedef struct {
	int x;
	float duration;
} AnimationFrame;

typedef enum {
	DIRECTION_FORWARD,
	DIRECTION_BACKWARDS,
	DIRECTION_PINGPONG
} AnimationDirection;

typedef struct {
	uint16_t start_frame;
	uint16_t end_frame;

	AnimationDirection direction;
} AnimationTag;

typedef struct {
	GPU_Image* texture;

	AnimationFrame* frames;
	int frame_count;
	std::map<std::string, AnimationTag>* tags;
} AnimationData;

void cleanup_animation_data(LitState* state, LitUserdata* d) {
	auto data = (AnimationData*) d->data;

	delete data->frames;
	delete data->tags;
}

LIT_METHOD(animation_data_constructor) {
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

	SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(pixels, w, h, 32, 4 * w, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);

	GPU_Image* texture = GPU_CopyImageFromSurface(surface);
	GPU_SetImageFilter(texture, GPU_FILTER_NEAREST);
	GPU_SetSnapMode(texture, GPU_SNAP_NONE);

	tsab_graphics_add_image(texture);

	AnimationData* data = LIT_INSERT_DATA(AnimationData, cleanup_animation_data);

	data->texture = texture;
	data->tags = new std::map<std::string, AnimationTag>();

	for (int i = 0; i < ase->tag_count; i++) {
		auto tag = ase->tags[i];

		(*data->tags)[tag.name] = (AnimationTag) {
			(uint16_t) tag.from_frame,
			(uint16_t) tag.to_frame,
			(AnimationDirection) tag.loop_animation_direction
		};
	}

	data->frame_count = ase->frame_count;
	data->frames = new AnimationFrame[data->frame_count];

	for (int i = 0; i < data->frame_count; i++) {
		data->frames[i] = (AnimationFrame) {
			i * ase->w,
			ase->frames[i].duration_milliseconds / 1000.0f
		};
	}

	cute_aseprite_free(ase);
	return instance;
}

void tsab_animation_bind_api(LitState* state) {
	LIT_BEGIN_CLASS("AnimationData")
		LIT_BIND_CONSTRUCTOR(animation_data_constructor)
	LIT_END_CLASS()
}