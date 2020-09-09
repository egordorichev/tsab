#ifndef TSAB_AUDIO_HPP
#define TSAB_AUDIO_HPP

#include <tsab/tsab_common.hpp>

void tsab_audio_init();
void tsab_audio_quit();

void tsab_audio_bind_api(LitState* state);

#endif