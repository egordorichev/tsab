#include <tsab/tsab.hpp>
#include <emscripten.h>

void loop_fn(void* arg) {
	if (tsab_frame()) {
		emscripten_cancel_main_loop();
	}
}

int main() {
	if (!tsab_init()) {
		return -1;
	}

	emscripten_set_main_loop_arg(loop_fn, nullptr, 60, 1);
	tsab_quit();

	return 0;
}