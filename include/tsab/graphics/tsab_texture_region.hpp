#ifndef TSAB_TEXTURE_REGION_HPP
#define TSAB_TEXTURE_REGION_HPP

#include <tsab/tsab_common.hpp>
#include "SDL_gpu.h"

typedef struct TextureRegion {
	GPU_Image* texture;

	uint16_t x;
	uint16_t y;
	uint16_t w;
	uint16_t h;
} TextureRegion;

void tsab_texture_region_bind_api(LitState* state);

#endif