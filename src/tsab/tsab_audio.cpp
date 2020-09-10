#include <tsab/tsab_audio.hpp>

#ifdef EMSCRIPTEN
#include <SDL/SDL_mixer.h>
#else
#include <SDL2/SDL_mixer.h>
#endif

#include <vector>
#include <map>
#include <string>
#include <iostream>

static std::vector<Mix_Chunk*> loaded_sounds;
static std::map<std::string, int> sound_ids;
static std::map<std::string, Mix_Music*> loaded_music;

void tsab_audio_init() {
	if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 1024) == -1) {
		return;
	}
}

void tsab_audio_quit() {
	for (Mix_Chunk* sound : loaded_sounds) {
		Mix_FreeChunk(sound);
	}

	for (auto & [key, value] : loaded_music) {
		Mix_FreeMusic(value);
	}

	Mix_CloseAudio();
}

LIT_METHOD(audio_new_sound) {
	const char* path = LIT_CHECK_STRING(0);
	std::string string_path = std::string(path);
	auto existing = sound_ids.find(string_path);

	if (existing != sound_ids.end()) {
		return NUMBER_VALUE(existing->second);
	}

	Mix_Chunk* sound = Mix_LoadWAV(path);

	if (sound == NULL) {
		return NULL_VALUE;
	}

	int id = loaded_sounds.size();

	sound_ids[string_path] = id;
	loaded_sounds.push_back(sound);

	return NUMBER_VALUE(id);
}

static void play_music(const char* path, float volume, float fade_in, bool looped) {
	std::string string_path = std::string(path);
	auto existing = loaded_music.find(string_path);
	Mix_Music* music = NULL;

	if (existing == loaded_music.end()) {
		music = Mix_LoadMUS(path);
	} else {
		music = existing->second;
	}

	if (music == NULL) {
		return;
	}

	if (volume >= 0) {
		Mix_VolumeMusic(MIX_MAX_VOLUME * volume);
	}

	int loops = looped ? -1 : 1;

	if (fade_in >= 0) {
		Mix_FadeInMusic(music, loops, fade_in * 1000);
	} else {
		Mix_PlayMusic(music, loops);
	}
}

LIT_METHOD(audio_fade_in) {
	const char* path = LIT_CHECK_STRING(0);
	float time = LIT_GET_NUMBER(1, 1);
	float volume = LIT_GET_NUMBER(2, -1);
	bool looped = LIT_GET_BOOL(3, true);

	play_music(path, volume, time, looped);
	return NULL_VALUE;
}

LIT_METHOD(audio_fade_out) {
	float time = LIT_GET_NUMBER(0, 1);
	Mix_FadeOutMusic(time * 1000);
	return NULL_VALUE;
}

LIT_METHOD(audio_play) {
	if (arg_count < 1) {
		return NULL_VALUE;
	}

	if (IS_STRING(args[0])) {
		// This is music

		const char* path = AS_CSTRING(args[0]);
		float volume = LIT_GET_NUMBER(1, -1);
		bool looped = LIT_GET_BOOL(2, true);

		play_music(path, volume, 0, looped);
	} else if (IS_NUMBER(args[0])) {
		// This is a sound effect
		int sfx_id = AS_NUMBER(args[0]);

		if (sfx_id < 0 || sfx_id >= sound_ids.size()) {
			return NULL_VALUE;
		}

		Mix_PlayChannel(-1, loaded_sounds[sfx_id], 0);
	}

	return NULL_VALUE;
}

void tsab_audio_bind_api(LitState* state) {
	LIT_BEGIN_CLASS("Audio")
		LIT_BIND_STATIC_METHOD("newSound", audio_new_sound)
		LIT_BIND_STATIC_METHOD("fadeIn", audio_fade_in)
		LIT_BIND_STATIC_METHOD("fadeOut", audio_fade_out)
		LIT_BIND_STATIC_METHOD("play", audio_play)
	LIT_END_CLASS()
}