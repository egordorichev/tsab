#ifndef TSAB_COMMON_HPP
#define TSAB_COMMON_HPP

#include <lit/lit.hpp>

#include <cstdint>
#include <cstdbool>

typedef uint32_t uint;

void tsab_report_sdl_error();
void tsab_report_sdl_error_non_fatal();

#endif