#include <tsab/tsab.hpp>

int main(int argc, const char** argv) {
	bool debug = false;

	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			const char* arg = argv[i];

			if (strcmp(arg, "-d") == 0 || strcmp(arg, "--debug") == 0) {
				debug = true;
			} else {
				printf("Unknown argument %s\n", arg);
				return 1;
			}
		}
	}

	if (!tsab_init(debug)) {
		return -1;
	}

	tsab_setup_loop();
	tsab_loop();
	tsab_quit();

	return 0;
}