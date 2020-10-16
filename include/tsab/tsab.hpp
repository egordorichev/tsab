#ifndef TSAB_HPP
#define TSAB_HPP

#include <tsab/tsab_common.hpp>

bool tsab_init(bool debug);
void tsab_quit();

void tsab_setup_loop();
void tsab_loop();
bool tsab_frame();

float tsab_get_dt();
LitState* tsab_get_state();

void tsab_error(const char* message);
void tsab_fatal_error(const char* message);

void tsab_call_method(const char* name, LitValue* args, uint arg_count);
bool tsab_handle_call(LitInterpretResult result);

#endif