#include <tsab/graphics/tsab_animation.hpp>

#define CUTE_ASEPRITE_IMPLEMENTATION
#include "cute_aseprite.h"

typedef struct {
	ase_t* ase;
} Animation;

static Animation* extract_data(LitState* state, LitValue instance) {
	LitValue data;

	if (!lit_table_get(&AS_INSTANCE(instance)->fields, CONST_STRING(state, "_data"), &data)) {
		return nullptr;
	}

	return (Animation*) AS_USERDATA(data)->data;
}

void cleanup_data(LitState* state, LitUserdata* d) {
	Animation* data = ((Animation*) d->data);

	if (data->ase != nullptr) {
		cute_aseprite_free(data->ase);
		data->ase = nullptr;

	}
}

LIT_METHOD(aseprite_constructor) {
	const char* path = LIT_CHECK_STRING(0);
	ase_t* ase = cute_aseprite_load_from_file(path, nullptr);

	if (ase == nullptr) {
		lit_runtime_error(vm, "Failed to open aseprite file %s", path);
	}

	LitUserdata* userdata = lit_create_userdata(vm->state, sizeof(Animation));
	userdata->cleanup_fn = cleanup_data;

	lit_table_set(vm->state, &AS_INSTANCE(instance)->fields, CONST_STRING(vm->state, "_data"), OBJECT_VALUE(userdata));
	
	auto data = (Animation*) userdata->data;
	data->ase = ase;

	return instance;
}

void tsab_animation_bind_api(LitState* state) {
	
}