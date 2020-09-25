#include <tsab/tsab_input.hpp>

#include <SDL.h>

#include <map>
#include <cstring>
#include <iostream>

static Uint8 *input_previous_gamepad_button_state;
static Uint8 *input_current_gamepad_button_state;
static Sint16 *input_gamepad_axis_state;
static Uint8 *input_previous_mouse_state;
static Uint8 *input_current_mouse_state;
static Uint8 *input_previous_keyboard_state;
static SDL_GameController *controller = nullptr;
static const Uint8 *input_current_keyboard_state;

extern "C" const char gamecontrollerdb[];
extern "C" const size_t gamecontrollerdb_len;

static std::map<std::string, int> key_map;

static void setup_key_map() {
	key_map["a"] = SDL_SCANCODE_A;
	key_map["b"] = SDL_SCANCODE_B;
	key_map["c"] = SDL_SCANCODE_C;
	key_map["d"] = SDL_SCANCODE_D;
	key_map["e"] = SDL_SCANCODE_E;
	key_map["f"] = SDL_SCANCODE_F;
	key_map["g"] = SDL_SCANCODE_G;
	key_map["h"] = SDL_SCANCODE_H;
	key_map["i"] = SDL_SCANCODE_I;
	key_map["j"] = SDL_SCANCODE_J;
	key_map["k"] = SDL_SCANCODE_K;
	key_map["l"] = SDL_SCANCODE_L;
	key_map["m"] = SDL_SCANCODE_M;
	key_map["n"] = SDL_SCANCODE_N;
	key_map["o"] = SDL_SCANCODE_O;
	key_map["p"] = SDL_SCANCODE_P;
	key_map["q"] = SDL_SCANCODE_Q;
	key_map["r"] = SDL_SCANCODE_R;
	key_map["s"] = SDL_SCANCODE_S;
	key_map["t"] = SDL_SCANCODE_T;
	key_map["u"] = SDL_SCANCODE_U;
	key_map["v"] = SDL_SCANCODE_V;
	key_map["w"] = SDL_SCANCODE_W;
	key_map["x"] = SDL_SCANCODE_X;
	key_map["y"] = SDL_SCANCODE_Y;
	key_map["z"] = SDL_SCANCODE_Z;
	key_map["0"] = SDL_SCANCODE_0;
	key_map["1"] = SDL_SCANCODE_1;
	key_map["2"] = SDL_SCANCODE_2;
	key_map["3"] = SDL_SCANCODE_3;
	key_map["4"] = SDL_SCANCODE_4;
	key_map["5"] = SDL_SCANCODE_5;
	key_map["6"] = SDL_SCANCODE_6;
	key_map["7"] = SDL_SCANCODE_7;
	key_map["8"] = SDL_SCANCODE_8;
	key_map["9"] = SDL_SCANCODE_9;
	key_map["f1"] = SDL_SCANCODE_F1;
	key_map["f2"] = SDL_SCANCODE_F2;
	key_map["f3"] = SDL_SCANCODE_F3;
	key_map["f4"] = SDL_SCANCODE_F4;
	key_map["f5"] = SDL_SCANCODE_F5;
	key_map["f6"] = SDL_SCANCODE_F6;
	key_map["f7"] = SDL_SCANCODE_F7;
	key_map["f8"] = SDL_SCANCODE_F8;
	key_map["f9"] = SDL_SCANCODE_F9;
	key_map["f10"] = SDL_SCANCODE_F10;
	key_map["f11"] = SDL_SCANCODE_F11;
	key_map["f12"] = SDL_SCANCODE_F12;
	key_map["space"] = SDL_SCANCODE_SPACE;
	key_map["lshift"] = SDL_SCANCODE_LSHIFT;
	key_map["rshift"] = SDL_SCANCODE_RSHIFT;
	key_map["rcontrol"] = SDL_SCANCODE_LCTRL;
	key_map["lcontrol"] = SDL_SCANCODE_RCTRL;
	key_map["lalt"] = SDL_SCANCODE_LALT;
	key_map["ralt"] = SDL_SCANCODE_RALT;
	key_map["capslock"] = SDL_SCANCODE_CAPSLOCK;
	key_map["tab"] = SDL_SCANCODE_TAB;
	key_map["escape"] = SDL_SCANCODE_ESCAPE;
	key_map["delete"] = SDL_SCANCODE_DELETE;
	key_map["backspace"] = SDL_SCANCODE_BACKSPACE;
	key_map["slash"] = SDL_SCANCODE_SLASH;
	key_map["bslash"] = SDL_SCANCODE_BACKSLASH;
	key_map["home"] = SDL_SCANCODE_HOME;
	key_map["end"] = SDL_SCANCODE_END;
	key_map["page_up"] = SDL_SCANCODE_PAGEUP;
	key_map["page_down"] = SDL_SCANCODE_PAGEDOWN;
	key_map["left"] = SDL_SCANCODE_LEFT;
	key_map["right"] = SDL_SCANCODE_RIGHT;
	key_map["up"] = SDL_SCANCODE_UP;
	key_map["down"] = SDL_SCANCODE_DOWN;
	key_map["comma"] = SDL_SCANCODE_COMMA;

	key_map["mouse1"] = MOUSE_1;
	key_map["mouse2"] = MOUSE_2;
	key_map["mouse3"] = MOUSE_3;
	key_map["mouse4"] = MOUSE_4;
	key_map["mouse5"] = MOUSE_5;
	key_map["mouse_wheel_up"] = MOUSE_WHEEL_UP;
	key_map["mouse_wheel_down"] = MOUSE_WHEEL_DOWN;
	key_map["mouse_wheel_left"] = MOUSE_WHEEL_LEFT;
	key_map["mouse_wheel_right"] = MOUSE_WHEEL_RIGHT;

	key_map["controller_x"] = SDL_CONTROLLER_BUTTON_X;
	key_map["controller_y"] = SDL_CONTROLLER_BUTTON_Y;
	key_map["controller_a"] = SDL_CONTROLLER_BUTTON_A;
	key_map["controller_b"] = SDL_CONTROLLER_BUTTON_B;
	key_map["controller_back"] = SDL_CONTROLLER_BUTTON_BACK;
	key_map["controller_start"] = SDL_CONTROLLER_BUTTON_START;
	key_map["controller_dpad_up"] = SDL_CONTROLLER_BUTTON_DPAD_UP;
	key_map["controller_dpad_down"] = SDL_CONTROLLER_BUTTON_DPAD_DOWN;
	key_map["controller_dpad_left"] = SDL_CONTROLLER_BUTTON_DPAD_LEFT;
	key_map["controller_dpad_right"] = SDL_CONTROLLER_BUTTON_DPAD_RIGHT;
	key_map["controller_lstick_button"] = SDL_CONTROLLER_BUTTON_LEFTSTICK;
	key_map["controller_rstick_button"] = SDL_CONTROLLER_BUTTON_LEFTSTICK;
	key_map["controller_lshoulder"] = SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
	key_map["controller_rshoulder"] = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;

	key_map["lx"] = SDL_CONTROLLER_AXIS_LEFTX;
	key_map["ry"] = SDL_CONTROLLER_AXIS_LEFTY;
	key_map["lx"] = SDL_CONTROLLER_AXIS_RIGHTX;
	key_map["ry"] = SDL_CONTROLLER_AXIS_RIGHTY;
	key_map["ltrigger"] = SDL_CONTROLLER_AXIS_TRIGGERLEFT;
	key_map["rtrigger"] = SDL_CONTROLLER_AXIS_TRIGGERRIGHT;
}

void tsab_input_init() {
	setup_key_map();
	SDL_GameControllerAddMappingsFromRW(SDL_RWFromMem((void *) gamecontrollerdb, gamecontrollerdb_len), 1);

	input_previous_keyboard_state = new Uint8[512];
	input_current_keyboard_state = SDL_GetKeyboardState(nullptr);

	input_previous_mouse_state = new Uint8[12];
	input_current_mouse_state = new Uint8[12];

	input_previous_gamepad_button_state = new Uint8[SDL_CONTROLLER_BUTTON_MAX - SDL_CONTROLLER_BUTTON_INVALID + 1];
	input_current_gamepad_button_state = new Uint8[SDL_CONTROLLER_BUTTON_MAX - SDL_CONTROLLER_BUTTON_INVALID + 1];
	input_gamepad_axis_state = new Sint16[SDL_CONTROLLER_AXIS_MAX - SDL_CONTROLLER_AXIS_INVALID + 1];
}

void tsab_input_quit() {
	delete input_previous_mouse_state;
	delete input_current_mouse_state;
	delete input_previous_keyboard_state;
}

void tsab_input_update() {
	for (int i = 0; i < 512; i++) {
		input_previous_keyboard_state[i] = input_current_keyboard_state[i];
	}

	for (int i = 0; i < 12; i++) {
		input_previous_mouse_state[i] = input_current_mouse_state[i];
		input_current_mouse_state[i] = 0;
	}

	for (int i = 0; i < 24; i++) {
		input_previous_gamepad_button_state[i] = input_current_gamepad_button_state[i];
	}

	if (controller != nullptr) {
		for (int i = SDL_CONTROLLER_BUTTON_INVALID; i < SDL_CONTROLLER_BUTTON_MAX; i++) {
			input_current_gamepad_button_state[i] = SDL_GameControllerGetButton(controller, (SDL_GameControllerButton) i);
		}

		for (int i = SDL_CONTROLLER_AXIS_INVALID; i < SDL_CONTROLLER_AXIS_MAX; i++) {
			input_gamepad_axis_state[i] = SDL_GameControllerGetAxis(controller, (SDL_GameControllerAxis) i);
		}
	}

	input_current_keyboard_state = SDL_GetKeyboardState(nullptr);
}

void tsab_input_handle_event(SDL_Event *event) {
	switch (event->type) {
		case SDL_MOUSEBUTTONDOWN: {
			if (event->button.button == SDL_BUTTON_LEFT) input_current_mouse_state[MOUSE_1] = 1;
			if (event->button.button == SDL_BUTTON_RIGHT) input_current_mouse_state[MOUSE_2] = 1;
			if (event->button.button == SDL_BUTTON_MIDDLE) input_current_mouse_state[MOUSE_3] = 1;
			if (event->button.button == SDL_BUTTON_X1) input_current_mouse_state[MOUSE_4] = 1;
			if (event->button.button == SDL_BUTTON_X2) input_current_mouse_state[MOUSE_5] = 1;

			break;
		}

		case SDL_MOUSEWHEEL: {
			if (event->wheel.x == 1) input_current_mouse_state[MOUSE_WHEEL_RIGHT] = 1;
			if (event->wheel.x == -1) input_current_mouse_state[MOUSE_WHEEL_LEFT] = 1;
			if (event->wheel.y == 1) input_current_mouse_state[MOUSE_WHEEL_DOWN] = 1;
			if (event->wheel.y == -1) input_current_mouse_state[MOUSE_WHEEL_UP] = 1;

			break;
		}

		case SDL_JOYDEVICEADDED:
		case SDL_CONTROLLERDEVICEADDED: {
			if (controller == nullptr) {
				controller = SDL_GameControllerOpen(event->cdevice.which);

				if (controller == nullptr) {
					std::cerr << "Failed to connect controller: " << SDL_GetError() << "\n";
				} else {
					std::cout << "Registered controller\n";
				}
			}

			break;
		}

		case SDL_JOYDEVICEREMOVED:
		case SDL_CONTROLLERDEVICEREMOVED: {
			if (controller == reinterpret_cast<SDL_GameController *>(event->cdevice.which)) {
				controller = nullptr;
				std::cout << "Removed controller\n";
			}

			break;
		}
	}
}

LIT_METHOD(input_get_axis) {
	if (controller == nullptr) {
		return FALSE_VALUE;
	}

	const char *axis = LIT_CHECK_STRING(0);
	auto it = key_map.find(axis);

	if (it != key_map.end()) {
		return NUMBER_VALUE(std::max<double>(-1, ((double) input_gamepad_axis_state[it->second]) / 32767));
	} else {
		std::cout << "No such axis " << axis << std::endl;
		return FALSE_VALUE;
	}
}

LIT_METHOD(input_mouse_x) {
	int x = 0;
	int y = 0;

	SDL_GetMouseState(&x, &y);
	return NUMBER_VALUE(x);
}

LIT_METHOD(input_mouse_y) {
	int x = 0;
	int y = 0;

	SDL_GetMouseState(&x, &y);
	return NUMBER_VALUE(y);
}

LIT_METHOD(input_was_released) {
	const char *key = LIT_CHECK_STRING(0);

	if (strstr(key, "controller") != nullptr) {
		if (controller == nullptr) {
			return FALSE_VALUE;
		}

		auto it = key_map.find(key);

		if (it != key_map.end()) {
			int scancode = it->second;

			return BOOL_VALUE(input_current_gamepad_button_state[scancode] == 0 && input_previous_gamepad_button_state[scancode] == 1);
		} else {
			std::cout << "No such control " << key << std::endl;
			return FALSE_VALUE;
		}
	} else if (strstr(key, "mouse") != nullptr) {
		auto it = key_map.find(key);

		if (it != key_map.end()) {
			int scancode = it->second;

			return BOOL_VALUE(input_current_mouse_state[scancode] == 0 && input_previous_mouse_state[scancode] == 1);
		} else {
			std::cout << "No such mouse control " << key << std::endl;
			return FALSE_VALUE;
		}
	} else {
		auto it = key_map.find(key);

		if (it != key_map.end()) {
			int scancode = it->second;
			return BOOL_VALUE(input_current_keyboard_state[scancode] == 0 && input_previous_keyboard_state[scancode] == 1);
		} else {
			std::cout << "No such key " << key << std::endl;
			return FALSE_VALUE;
		}
	}

	return FALSE_VALUE;
}

LIT_METHOD(input_is_down) {
	const char *key = LIT_CHECK_STRING(0);

	if (strstr(key, "controller") != nullptr) {
		if (controller == nullptr) {
			return FALSE_VALUE;
		}

		auto it = key_map.find(key);

		if (it != key_map.end()) {
			return BOOL_VALUE(input_current_gamepad_button_state[it->second] == 1);
		} else {
			std::cout << "No such control " << key << std::endl;
			return FALSE_VALUE;
		}
	} else if (strstr(key, "mouse") != nullptr) {
		auto it = key_map.find(key);

		if (it != key_map.end()) {
			return BOOL_VALUE(input_current_mouse_state[it->second] == 1);
		} else {
			std::cout << "No such mouse control " << key << std::endl;
			return FALSE_VALUE;
		}
	} else {
		auto it = key_map.find(key);

		if (it != key_map.end()) {
			return BOOL_VALUE(input_current_keyboard_state[it->second] == 1);
		} else {
			std::cout << "No such key " << key << std::endl;
			return FALSE_VALUE;
		}
	}

	return FALSE_VALUE;
}

LIT_METHOD(input_was_pressed) {
	const char *key = LIT_CHECK_STRING(0);

	if (strstr(key, "controller") != nullptr) {
		if (controller == nullptr) {
			return FALSE_VALUE;
		}

		auto it = key_map.find(key);

		if (it != key_map.end()) {
			int scancode = it->second;
			return BOOL_VALUE(input_current_gamepad_button_state[scancode] == 1 && input_previous_gamepad_button_state[scancode] == 0);
		} else {
			std::cout << "No such control " << key << std::endl;
			return FALSE_VALUE;
		}
	} else if (strstr(key, "mouse") != nullptr) {
		auto it = key_map.find(key);

		if (it != key_map.end()) {
			int scancode = it->second;
			return BOOL_VALUE(input_current_mouse_state[scancode] == 1 && input_previous_mouse_state[scancode] == 0);
		} else {
			std::cout << "No such mouse control " << key << std::endl;
			return FALSE_VALUE;
		}
	} else {
		auto it = key_map.find(key);

		if (it != key_map.end()) {
			int scancode = it->second;
			return BOOL_VALUE(input_current_keyboard_state[scancode] == 1 && input_previous_keyboard_state[scancode] == 0);
		} else {
			std::cout << "No such key " << key << std::endl;
			return FALSE_VALUE;
		}
	}

	return FALSE_VALUE;
}

void tsab_input_bind_api(LitState* state) {
	LIT_BEGIN_CLASS("Input")
		LIT_BIND_STATIC_GETTER("mouseX", input_mouse_x);
		LIT_BIND_STATIC_GETTER("mouseY", input_mouse_y);

		LIT_BIND_STATIC_METHOD("isDown", input_is_down);
		LIT_BIND_STATIC_METHOD("wasReleased", input_was_released);
		LIT_BIND_STATIC_METHOD("wasPressed", input_was_pressed);
		LIT_BIND_STATIC_METHOD("getAxis", input_get_axis);
	LIT_END_CLASS()
}