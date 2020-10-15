#include <tsab/graphics/tsab_tilemap.hpp>
#include <tsab/graphics/tsab_graphics.hpp>

#define CUTE_TILED_IMPLEMENTATION
#define STRPOOL_EMBEDDED_IMPLEMENTATION

#include "cute_tiled.h"
#include "SDL_gpu.h"

#include <math.h>

typedef struct {
	GPU_Image* texture;

	cute_tiled_map_t* map;
	cute_tiled_layer_t* tiles;
} Tilemap;

void cleanup_tilemap(LitState* state, LitUserdata* d, bool mark) {
	if (mark) {
		return;
	}

	auto map = (Tilemap*) d->data;
	cute_tiled_free_map(map->map);
}

LIT_METHOD(tilemap_constructor) {
	const char* path = LIT_CHECK_STRING(0);
	const char* layer_name = LIT_CHECK_STRING(1);

	cute_tiled_map_t* map = cute_tiled_load_map_from_file(path, nullptr);

	if (map == nullptr) {
		lit_runtime_error_exiting(vm, "Failed to load map %s", path);
	}

	cute_tiled_layer_t* tiles = map->layers;

	while (tiles != nullptr) {
		if (tiles->data != nullptr && strcmp(tiles->name.ptr, layer_name) == 0) {
			break;
		}

		tiles = tiles->next;
	}

	if (tiles == nullptr) {
		lit_runtime_error_exiting(vm, "Failed to find tile layer %s", layer_name);
	}

	Tilemap* data = LIT_INSERT_DATA(Tilemap, nullptr);

	data->map = map;
	data->tiles = tiles;

	// Complex path manipulation, basically a/b/map.json -> a/b/tiles.png
	const char* index = strrchr(path, '/');
	auto tileset_name = map->tilesets->name.ptr;

	int path_length = index == nullptr ? 0 : strlen(path) - strlen(index) + 1;
	int ending_length = strlen(tileset_name);

	char texture_path[path_length + ending_length + 5];

	memcpy(texture_path, path, path_length);
	memcpy(texture_path + path_length, tileset_name, ending_length);
	memcpy(texture_path + path_length + ending_length, ".png\0", 5);

	auto texture = GPU_LoadImage(texture_path);

	if (texture == nullptr) {
		lit_runtime_error_exiting(vm, "Failed to find tile texture %s", texture_path);
	}

	data->texture = texture;

	GPU_SetImageFilter(texture, GPU_FILTER_NEAREST);
	GPU_SetAnchor(texture, 0, 0); // Aka origin
	tsab_graphics_add_image(texture);

	return instance;
}

LIT_METHOD(tilemap_render) {
	auto target = tsab_graphics_get_current_target();

	auto tilemap = LIT_EXTRACT_DATA(Tilemap);

	// Put as much heavy-accessed variables into locals as we can for speed increase
	auto texture = tilemap->texture;
	auto map = tilemap->map;
	auto data = tilemap->tiles->data;
	auto tileset = map->tilesets;

	int tw = tileset->tilewidth;
	int th = tileset->tileheight;
	int c = tileset->columns;

	int x = fmax(0, fmin(map->width, LIT_GET_NUMBER(0, 0)));
	int y = fmax(0, fmin(map->height, LIT_GET_NUMBER(1, 0)));
	int w = fmax(0, fmin(map->width - x, LIT_GET_NUMBER(2, map->width)));
	int h = fmax(0, fmin(map->height - y, LIT_GET_NUMBER(3, map->height)));

	for (int ty = y; ty < y + h; ty++) {
		for (int tx = x; tx < x + w; tx++) {
			int tile = data[tx + ty * map->width];

			if (tile == 0) {
				continue;
			}

			tile--;
			GPU_Rect r = GPU_MakeRect(tile % c * tw, tile / c * th, tw, th);
			GPU_Blit(texture, &r, target, tx * tw, ty * th);
		}
	}

	return NULL_VALUE;
}

void tsab_tilemap_bind_api(LitState* state) {
	LIT_BEGIN_CLASS("Tilemap")
		LIT_BIND_CONSTRUCTOR(tilemap_constructor)

		LIT_BIND_METHOD("render", tilemap_render)
	LIT_END_CLASS()
}