#include <tsab/graphics/tsab_texture_region.hpp>
#include <tsab/graphics/tsab_graphics.hpp>

LIT_METHOD(texture_region_constructor) {
	LIT_ENSURE_ARGS(5)

	GPU_Image* texture = nullptr;

	if (IS_NUMBER(args[0])) {
		texture = tsab_graphics_get_image(AS_NUMBER(args[0]));
	}

	if (texture == nullptr) {
		lit_runtime_error_exiting(vm, "Unknown texture");
	}

	TextureRegion* data = LIT_INSERT_DATA(TextureRegion, nullptr);

	data->x = LIT_CHECK_NUMBER(1);
	data->y = LIT_CHECK_NUMBER(2);
	data->w = LIT_CHECK_NUMBER(3);
	data->h = LIT_CHECK_NUMBER(4);
	data->texture = texture;

	return instance;
}

LIT_METHOD(texture_region_x) {
	TextureRegion* region = LIT_EXTRACT_DATA(TextureRegion);

	if (arg_count == 0) {
		return NUMBER_VALUE(region->x);
	}

	region->x = LIT_CHECK_NUMBER(0);
	return args[0];
}

LIT_METHOD(texture_region_y) {
	TextureRegion* region = LIT_EXTRACT_DATA(TextureRegion);

	if (arg_count == 0) {
		return NUMBER_VALUE(region->y);
	}

	region->y = LIT_CHECK_NUMBER(0);
	return args[0];
}

LIT_METHOD(texture_region_w) {
	TextureRegion* region = LIT_EXTRACT_DATA(TextureRegion);

	if (arg_count == 0) {
		return NUMBER_VALUE(region->w);
	}

	region->w = LIT_CHECK_NUMBER(0);
	return args[0];
}

LIT_METHOD(texture_region_h) {
	TextureRegion* region = LIT_EXTRACT_DATA(TextureRegion);

	if (arg_count == 0) {
		return NUMBER_VALUE(region->h);
	}

	region->h = LIT_CHECK_NUMBER(0);
	return args[0];
}

void tsab_texture_region_bind_api(LitState* state) {
	LIT_BEGIN_CLASS("TextureRegion")
		LIT_BIND_CONSTRUCTOR(texture_region_constructor)

		LIT_BIND_FIELD("x", texture_region_x, texture_region_x)
		LIT_BIND_FIELD("y", texture_region_y, texture_region_y)
		LIT_BIND_FIELD("w", texture_region_w, texture_region_w)
		LIT_BIND_FIELD("h", texture_region_h, texture_region_h)
	LIT_END_CLASS()
}