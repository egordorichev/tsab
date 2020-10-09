#include <tsab/tsab_input.hpp>

#include <SDL.h>

#include <map>
#include <cstring>
#include <iostream>
#include <tsab/tsab.hpp>

static Uint8 *input_previous_gamepad_button_state = nullptr;
static Uint8 *input_current_gamepad_button_state = nullptr;
static Sint16 *input_gamepad_axis_state = nullptr;
static Uint8 *input_previous_mouse_state = nullptr;
static Uint8 *input_current_mouse_state = nullptr;
static Uint8 *input_previous_keyboard_state = nullptr;
static SDL_GameController *controller = nullptr;
static const Uint8 *input_current_keyboard_state = nullptr;

extern "C" const char gamecontrollerdb[];
extern "C" const size_t gamecontrollerdb_len;

static std::map<std::string, int> key_map;
static std::map<int, std::string> key_names;

static void register_key(std::string key, SDL_Scancode code) {
	key_map[key] = code;
	key_names[code] = key;
}

static void setup_key_map() {
	register_key("a", SDL_SCANCODE_A);
	register_key("b", SDL_SCANCODE_B);
	register_key("c", SDL_SCANCODE_C);
	register_key("d", SDL_SCANCODE_D);
	register_key("e", SDL_SCANCODE_E);
	register_key("f", SDL_SCANCODE_F);
	register_key("g", SDL_SCANCODE_G);
	register_key("h", SDL_SCANCODE_H);
	register_key("i", SDL_SCANCODE_I);
	register_key("j", SDL_SCANCODE_J);
	register_key("k", SDL_SCANCODE_K);
	register_key("l", SDL_SCANCODE_L);
	register_key("m", SDL_SCANCODE_M);
	register_key("n", SDL_SCANCODE_N);
	register_key("o", SDL_SCANCODE_O);
	register_key("p", SDL_SCANCODE_P);
	register_key("q", SDL_SCANCODE_Q);
	register_key("r", SDL_SCANCODE_R);
	register_key("s", SDL_SCANCODE_S);
	register_key("t", SDL_SCANCODE_T);
	register_key("u", SDL_SCANCODE_U);
	register_key("v", SDL_SCANCODE_V);
	register_key("w", SDL_SCANCODE_W);
	register_key("x", SDL_SCANCODE_X);
	register_key("y", SDL_SCANCODE_Y);
	register_key("z", SDL_SCANCODE_Z);
	register_key("0", SDL_SCANCODE_0);
	register_key("1", SDL_SCANCODE_1);
	register_key("2", SDL_SCANCODE_2);
	register_key("3", SDL_SCANCODE_3);
	register_key("4", SDL_SCANCODE_4);
	register_key("5", SDL_SCANCODE_5);
	register_key("6", SDL_SCANCODE_6);
	register_key("7", SDL_SCANCODE_7);
	register_key("8", SDL_SCANCODE_8);
	register_key("9", SDL_SCANCODE_9);
	register_key("f1", SDL_SCANCODE_F1);
	register_key("f2", SDL_SCANCODE_F2);
	register_key("f3", SDL_SCANCODE_F3);
	register_key("f4", SDL_SCANCODE_F4);
	register_key("f5", SDL_SCANCODE_F5);
	register_key("f6", SDL_SCANCODE_F6);
	register_key("f7", SDL_SCANCODE_F7);
	register_key("f8", SDL_SCANCODE_F8);
	register_key("f9", SDL_SCANCODE_F9);
	register_key("f10", SDL_SCANCODE_F10);
	register_key("f11", SDL_SCANCODE_F11);
	register_key("f12", SDL_SCANCODE_F12);
	register_key("space", SDL_SCANCODE_SPACE);
	register_key("lshift", SDL_SCANCODE_LSHIFT);
	register_key("rshift", SDL_SCANCODE_RSHIFT);
	register_key("rcontrol", SDL_SCANCODE_LCTRL);
	register_key("lcontrol", SDL_SCANCODE_RCTRL);
	register_key("lalt", SDL_SCANCODE_LALT);
	register_key("ralt", SDL_SCANCODE_RALT);
	register_key("capslock", SDL_SCANCODE_CAPSLOCK);
	register_key("tab", SDL_SCANCODE_TAB);
	register_key("escape", SDL_SCANCODE_ESCAPE);
	register_key("delete", SDL_SCANCODE_DELETE);
	register_key("backspace", SDL_SCANCODE_BACKSPACE);
	register_key("slash", SDL_SCANCODE_SLASH);
	register_key("bslash", SDL_SCANCODE_BACKSLASH);
	register_key("home", SDL_SCANCODE_HOME);
	register_key("end", SDL_SCANCODE_END);
	register_key("page_up", SDL_SCANCODE_PAGEUP);
	register_key("page_down", SDL_SCANCODE_PAGEDOWN);
	register_key("left", SDL_SCANCODE_LEFT);
	register_key("right", SDL_SCANCODE_RIGHT);
	register_key("up", SDL_SCANCODE_UP);
	register_key("down", SDL_SCANCODE_DOWN);
	register_key("comma", SDL_SCANCODE_COMMA);

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
	if (input_previous_mouse_state != nullptr) {
		delete input_previous_mouse_state;
	}

	if (input_current_mouse_state != nullptr) {
		delete input_current_mouse_state;
	}

	if (input_previous_gamepad_button_state != nullptr) {
		delete input_previous_keyboard_state;
	}
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
					tsab_call_method("controllerConnected", nullptr, 0);
				}
			}

			break;
		}

		case SDL_JOYDEVICEREMOVED:
		case SDL_CONTROLLERDEVICEREMOVED: {
			if (controller == reinterpret_cast<SDL_GameController *>(event->cdevice.which)) {
				controller = nullptr;
				std::cout << "Removed controller\n";
				tsab_call_method("controllerDisconnected", nullptr, 0);
			}

			break;
		}

		case SDL_KEYDOWN: {
			auto name = key_names.find(event->key.keysym.scancode);

			if (name != key_names.end()) {
				LitValue key = OBJECT_VALUE(lit_copy_string(tsab_get_state(), name->second.c_str(), name->second.length()));
				tsab_call_method("keyPressed", &key, 1);
			}

			break;
		}

		case SDL_KEYUP: {
			auto name = key_names.find(event->key.keysym.scancode);

			if (name != key_names.end()) {
				LitValue key = OBJECT_VALUE(lit_copy_string(tsab_get_state(), name->second.c_str(), name->second.length()));
				tsab_call_method("keyReleased", &key, 1);
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