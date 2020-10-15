#include <tsab/tsab.hpp>

#include <emscripten.h>
#include <stdio.h>

void loop_fn(void* arg) {
	if (tsab_frame()) {
		emscripten_cancel_main_loop();
		printf("Exiting.\n");
	}
}

int main() {
	if (!tsab_init(false)) {
		printf("F.\n");
		return -1;
	}

	tsab_setup_loop();
	emscripten_set_main_loop_arg(loop_fn, nullptr, 0, 1);
	tsab_quit();

	return 0;
}