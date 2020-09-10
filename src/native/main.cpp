#include <tsab/tsab.hpp>

int main() {
	if (!tsab_init()) {
		return -1;
	}

	tsab_setup_loop();
	tsab_loop();
	tsab_quit();

	return 0;
}