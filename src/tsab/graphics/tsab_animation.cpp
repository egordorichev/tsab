#include <tsab/graphics/tsab_animation.hpp>
#include <tsab/graphics/tsab_graphics.hpp>
#include <tsab/graphics/tsab_texture_region.hpp>

#define CUTE_ASEPRITE_IMPLEMENTATION
#include "cute_aseprite.h"

#include "SDL_gpu.h"

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


struct char_cmp {
	bool operator () (const char* a, const char* b) const {
		return strcmp(a, b) < 0;
	}
};

typedef struct {
	GPU_Image* texture;
	int texture_id;

	AnimationFrame* frames;
	int frame_count;
	int width;
	int height;

	std::map<char*, AnimationTag, char_cmp>* tags;
	std::map<char*, TextureRegion, char_cmp>* slices;

	LitValue instance;
} AnimationData;

void cleanup_animation_data(LitState* state, LitUserdata* d, bool mark) {
	if (mark) {
		return;
	}

	auto data = (AnimationData*) d->data;

	for (auto & tag : *data->tags) {
		delete tag.first;
	}

	delete data->frames;
	delete data->tags;
	delete data->slices;
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

	AnimationData* data = LIT_INSERT_DATA(AnimationData, cleanup_animation_data);

	data->instance = instance;
	data->texture_id = tsab_graphics_add_image(texture);
	data->texture = texture;
	data->tags = new std::map<char*, AnimationTag, char_cmp>();
	data->slices = new std::map<char*, TextureRegion, char_cmp>();
	data->width = ase->w;
	data->height = ase->h;
	data->frame_count = ase->frame_count;
	data->frames = new AnimationFrame[data->frame_count];

	for (int i = 0; i < ase->tag_count; i++) {
		auto tag = ase->tags[i];

		int length = strlen(tag.name) + 1;
		char* str = new char[length];
		memcpy(str, tag.name, length);

		(*data->tags)[str] = (AnimationTag) {
			(uint16_t) tag.from_frame,
			(uint16_t) tag.to_frame,
			(AnimationDirection) tag.loop_animation_direction
		};
	}

	for (int i = 0; i < data->frame_count; i++) {
		data->frames[i] = (AnimationFrame) {
			i * ase->w,
			ase->frames[i].duration_milliseconds / 1000.0f
		};
	}

	for (int i = 0; i < ase->slice_count; i++) {
		auto slice = ase->slices[i];

		int length = strlen(slice.name) + 1;
		char* str = new char[length];
		memcpy(str, slice.name, length);

		(*data->slices)[str] = (TextureRegion) {
			data->texture,
			(uint16_t) slice.origin_x,
			(uint16_t) slice.origin_y,
			(uint16_t) slice.w,
			(uint16_t) slice.h
		};
	}

	cute_aseprite_free(ase);
	return instance;
}

LIT_METHOD(animation_data_get_slice) {
	const char* name = LIT_CHECK_STRING(0);
	auto data = LIT_EXTRACT_DATA(AnimationData);

	auto iterator = data->slices->find((char*) name);

	if (iterator == data->slices->end()) {
		return NULL_VALUE;
	}

	auto slice = iterator->second;

	LitValue ar[5] {
		NUMBER_VALUE(data->texture_id),
		NUMBER_VALUE(slice.x),
		NUMBER_VALUE(slice.y),
		NUMBER_VALUE(slice.w),
		NUMBER_VALUE(slice.h),
	};

	return lit_call_new(vm, "TextureRegion", ar, 5);
}

/*
 * Actual animation holder
 */

typedef struct {
	AnimationData* data;
	AnimationFrame* frame;
	LitValue region;

	int frame_id;
	int start_frame;
	int end_frame;

	char* tag;
	float time;
} Animation;

static int interpolate_frame(int frame, int start, int end, AnimationDirection direction) {
	switch (direction) {
		case DIRECTION_FORWARD: {
			return start + frame;
		}

		case DIRECTION_BACKWARDS: {
			return end - frame;
		}

		case DIRECTION_PINGPONG: {
			// todo: implement properly
			return start + frame;
		}
	}

	return 0;
}

static void setup_frame_info(LitVm* vm, Animation* animation) {
	if (animation->tag == nullptr) {
		animation->start_frame = 0;
		animation->end_frame = animation->data->frame_count - 1;
		animation->frame = &animation->data->frames[interpolate_frame(animation->frame_id, animation->start_frame, animation->end_frame, DIRECTION_FORWARD)];

		return;
	}

	auto iterator = animation->data->tags->find(animation->tag);

	if (iterator == animation->data->tags->end()) {
		lit_runtime_error_exiting(vm, "Unknown animation tag '%s'", animation->tag);
	}

	AnimationTag tag = iterator->second;

	animation->start_frame = tag.start_frame;
	animation->end_frame = tag.end_frame;
	animation->frame = &animation->data->frames[interpolate_frame(animation->frame_id, animation->start_frame, animation->end_frame, tag.direction)];
}

void cleanup_animation(LitState* state, LitUserdata* d, bool mark) {
	if (!mark) {
		return;
	}

	auto animation = (Animation*) d->data;

	lit_mark_value(state->vm, animation->data->instance);
	lit_mark_value(state->vm, animation->region);
}

LIT_METHOD(animation_constructor) {
	LitInstance* data = LIT_CHECK_INSTANCE(0);

	if (strcmp(data->klass->name->chars, "AnimationData") != 0) {
		lit_runtime_error_exiting(vm, "Expected AnimationData as argument #1");
	}

	AnimationData* animation_data = LIT_EXTRACT_DATA_FROM(OBJECT_VALUE(data), AnimationData);
	Animation* animation = LIT_INSERT_DATA(Animation, cleanup_animation);

	animation->data = animation_data;
	animation->time = 0;
	animation->frame_id = 0;
	animation->region = NULL_VALUE;

	if (animation->data->tags->size() > 0) {
		animation->tag = animation->data->tags->begin()->first;
	} else {
		animation->tag = nullptr;
	}

	if (animation_data->frame_count > 0) {
		animation->frame = &animation_data->frames[0];
	} else {
		lit_runtime_error_exiting(vm, "AnimationData has 0 frames");
	}

	setup_frame_info(vm, animation);
	return instance;
}

LIT_METHOD(animation_update) {
	float dt = LIT_CHECK_NUMBER(0);

	Animation* animation = LIT_EXTRACT_DATA(Animation);
	animation->time += dt;

	if (animation->time >= animation->frame->duration) {
		animation->time = 0;
		animation->frame_id++;

		if (animation->frame_id >= (animation->end_frame - animation->start_frame)) {
			animation->frame_id = 0;
		}

		setup_frame_info(vm, animation);
	}

	return NULL_VALUE;
}

LIT_METHOD(animation_texture) {
	return NUMBER_VALUE(LIT_EXTRACT_DATA(Animation)->data->texture_id);
}

LIT_METHOD(animation_width) {
	return NUMBER_VALUE(LIT_EXTRACT_DATA(Animation)->data->width);
}

LIT_METHOD(animation_height) {
	return NUMBER_VALUE(LIT_EXTRACT_DATA(Animation)->data->height);
}

LIT_METHOD(animation_frame) {
	auto animation = LIT_EXTRACT_DATA(Animation);

	if (animation->region == NULL_VALUE) {
		LitValue ar[5] = {
			NUMBER_VALUE(animation->data->texture_id),
			NUMBER_VALUE(animation->frame->x),
			NUMBER_VALUE(0),
			NUMBER_VALUE(animation->data->width),
			NUMBER_VALUE(animation->data->height)
		};

		animation->region = lit_call_new(vm, "TextureRegion", ar, 5);
		return animation->region;
	}

	auto animation_data = LIT_EXTRACT_DATA_FROM(animation->region, TextureRegion);
	animation_data->x = animation->frame->x;

	return animation->region;
}

LIT_METHOD(animation_tag) {
	Animation* animation = LIT_EXTRACT_DATA(Animation);

	if (arg_count == 0) {
		return OBJECT_CONST_STRING(vm->state, animation->tag);
	}

	char* tag = (char*) LIT_CHECK_STRING(0);

	if (strcmp(tag, animation->tag) != 0) {
		animation->tag = tag;
		setup_frame_info(vm, animation);
	}

	return args[0];
}

void tsab_animation_bind_api(LitState* state) {
	LIT_BEGIN_CLASS("AnimationData")
		LIT_BIND_CONSTRUCTOR(animation_data_constructor)

		LIT_BIND_METHOD("getSlice", animation_data_get_slice)
	LIT_END_CLASS()


	LIT_BEGIN_CLASS("Animation")
		LIT_BIND_CONSTRUCTOR(animation_constructor)

		LIT_BIND_METHOD("update", animation_update)
		LIT_BIND_GETTER("texture", animation_texture)
		LIT_BIND_GETTER("width", animation_width)
		LIT_BIND_GETTER("height", animation_height)
		LIT_BIND_GETTER("frame", animation_frame)

		LIT_BIND_FIELD("tag", animation_tag, animation_tag)
	LIT_END_CLASS()
}