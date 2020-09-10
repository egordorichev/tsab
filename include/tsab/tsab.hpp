#ifndef TSAB_HPP
#define TSAB_HPP

bool tsab_init();
void tsab_quit();

void tsab_setup_loop();
void tsab_loop();
bool tsab_frame();

float tsab_get_dt();

#endif