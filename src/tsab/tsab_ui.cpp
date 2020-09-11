#include <tsab/tsab_ui.hpp>

#include <SDL2/SDL.h>
#include <SDL_GPU/SDL_gpu.h>

#include <imgui/imgui.h>
#include <imgui/examples/imgui_impl_sdl.h>
#include <imgui/examples/imgui_impl_opengl3.h>

void tsab_ui_init() {
	#if __APPLE__
		// GL 3.2 Core + GLSL 150
	  const char* glsl_version = "#version 150";

	  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
	  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	#else
		// GL 3.0 + GLSL 130
		const char* glsl_version = "#version 130";

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	#endif

	ImGui::CreateContext();
	ImGui_ImplSDL2_InitForOpenGL(SDL_GL_GetCurrentWindow(), SDL_GL_GetCurrentContext());
	ImGui_ImplOpenGL3_Init(glsl_version);
	ImGui::StyleColorsDark();

	ImGui::GetStyle().FrameRounding = 5;
}

void tsab_ui_quit() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}

LIT_METHOD(ui_new_frame) {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(SDL_GL_GetCurrentWindow());
	ImGui::NewFrame();

	return NULL_VALUE;
}

LIT_METHOD(ui_render) {
	GPU_FlushBlitBuffer();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	return NULL_VALUE;
}

LIT_METHOD(ui_set_next_window_position) {
	int x = LIT_CHECK_NUMBER(0);
	int y = LIT_CHECK_NUMBER(1);
	bool once = LIT_GET_BOOL(2, false);

	ImGui::SetNextWindowPos(ImVec2(x, y), once ? ImGuiCond_Once : ImGuiCond_Always);
	return NULL_VALUE;
}

LIT_METHOD(ui_set_next_window_size) {
	int x = LIT_CHECK_NUMBER(0);
	int y = LIT_CHECK_NUMBER(1);
	bool once = LIT_GET_BOOL(2, false);

	ImGui::SetNextWindowSize(ImVec2(x, y), once ? ImGuiCond_Once : ImGuiCond_Always);
	return NULL_VALUE;
}

LIT_METHOD(ui_begin) {
	ImGui::SetNextWindowBgAlpha(0.8f);
	const char* name = LIT_GET_STRING(0, "Easter Egg");

	if (arg_count == 1) {
		return BOOL_VALUE(ImGui::Begin(name));
	}

	ImGuiWindowFlags flags = 0;

	for (uint i = 1; i < arg_count; i++) {
		if (IS_STRING(args[i])) {
			char* flag = AS_CSTRING(args[i]);

			if (memcmp(flag, "noTitleBar", 10) == 0) {
				flags |= ImGuiWindowFlags_NoTitleBar;
			} else if (memcmp(flag, "noResize", 8) == 0) {
				flags |= ImGuiWindowFlags_NoResize;
			} else if (memcmp(flag, "noMove", 6) == 0) {
				flags |= ImGuiWindowFlags_NoMove;
			} else if (memcmp(flag, "noScrollbar", 11) == 0) {
				flags |= ImGuiWindowFlags_NoScrollbar;
			} else if (memcmp(flag, "noCollapse", 10) == 0) {
				flags |= ImGuiWindowFlags_NoCollapse;
			} else if (memcmp(flag, "autoResize", 10) == 0) {
				flags |= ImGuiWindowFlags_AlwaysAutoResize;
			}
		}
	}

	return BOOL_VALUE(ImGui::Begin(name, nullptr, flags));
}

LIT_METHOD(ui_end) {
	ImGui::End();
	return NULL_VALUE;
}

LIT_METHOD(ui_tree_node) {
	const char* name = LIT_GET_STRING(0, "Free muffins");
	return BOOL_VALUE(ImGui::TreeNode(name));
}

LIT_METHOD(ui_tree_pop) {
	ImGui::TreePop();
	return NULL_VALUE;
}

LIT_METHOD(ui_begin_popup) {
	const char* name = LIT_GET_STRING(0, "Never gonna give you up");
	return BOOL_VALUE(ImGui::BeginPopupModal(name, nullptr, ImGuiWindowFlags_AlwaysAutoResize));
}

LIT_METHOD(ui_end_popup) {
	ImGui::EndPopup();
	return NULL_VALUE;
}

LIT_METHOD(ui_open_popup) {
	const char* name = LIT_GET_STRING(0, "Never gonna give you up");
	ImGui::OpenPopup(name);

	return NULL_VALUE;
}

LIT_METHOD(ui_close_popup) {
	ImGui::CloseCurrentPopup();
	return NULL_VALUE;
}


LIT_METHOD(ui_text) {
	const char* text = LIT_CHECK_STRING(0);
	ImGui::Text(text);

	return NULL_VALUE;
}

LIT_METHOD(ui_bullet_text) {
	const char* text = LIT_CHECK_STRING(0);
	ImGui::BulletText(text);

	return NULL_VALUE;
}

LIT_METHOD(ui_button) {
	const char* text = LIT_CHECK_STRING(0);
	return BOOL_VALUE(ImGui::Button(text));
}

LIT_METHOD(ui_checkbox) {
	const char* text = LIT_CHECK_STRING(0);
	LitValue* value = LIT_CHECK_REFERENCE(1);

	lit_ensure_bool(vm, *value, "Expected a bool referenced");

	bool v = AS_BOOL(*value);
	bool result = ImGui::Checkbox(text, &v);

	if (result) {
		*value = BOOL_VALUE(v);
	}

	return BOOL_VALUE(result);
}

LIT_METHOD(ui_input_int) {
	const char* text = LIT_CHECK_STRING(0);
	LitValue* value = LIT_CHECK_REFERENCE(1);
	int step = LIT_GET_NUMBER(2, 1);

	lit_ensure_number(vm, *value, "Expected a number referenced");

	int v = AS_NUMBER(*value);
	bool result = ImGui::InputInt(text, &v, step);

	if (result) {
		*value = NUMBER_VALUE(v);
	}

	return BOOL_VALUE(result);
}

LIT_METHOD(ui_input_float) {
	const char* text = LIT_CHECK_STRING(0);
	LitValue* value = LIT_CHECK_REFERENCE(1);
	float step = LIT_GET_NUMBER(2, 1);

	lit_ensure_number(vm, *value, "Expected a number referenced");

	float v = AS_NUMBER(*value);
	bool result = ImGui::InputFloat(text, &v, step);

	if (result) {
		*value = NUMBER_VALUE(v);
	}

	return BOOL_VALUE(result);
}

LIT_METHOD(ui_drag_int) {
	const char* text = LIT_CHECK_STRING(0);
	LitValue* value = LIT_CHECK_REFERENCE(1);
	int step = LIT_GET_NUMBER(2, 1);

	lit_ensure_number(vm, *value, "Expected a number referenced");

	int v = AS_NUMBER(*value);
	bool result = ImGui::DragInt(text, &v, step);

	if (result) {
		*value = NUMBER_VALUE(v);
	}

	return BOOL_VALUE(result);
}

LIT_METHOD(ui_drag_float) {
	const char* text = LIT_CHECK_STRING(0);
	LitValue* value = LIT_CHECK_REFERENCE(1);
	float step = LIT_GET_NUMBER(2, 1);

	lit_ensure_number(vm, *value, "Expected a number referenced");

	float v = AS_NUMBER(*value);
	bool result = ImGui::DragFloat(text, &v, step);

	if (result) {
		*value = NUMBER_VALUE(v);
	}

	return BOOL_VALUE(result);
}

LIT_METHOD(ui_input_int2) {
	LIT_ENSURE_ARGS(2)

	const char* text = LIT_CHECK_STRING(0);
	LitValue value = args[1];

	if (IS_INSTANCE(value)) {
		LitInstance* inst = AS_INSTANCE(value);
		LitState* state = vm->state;
		LitValue field;

		int v[2];

		if (lit_table_get(&inst->fields, CONST_STRING(state, "x"), &field) && IS_NUMBER(field)) {
			v[0] = AS_NUMBER(field);
		}

		if (lit_table_get(&inst->fields, CONST_STRING(state, "y"), &field) && IS_NUMBER(field)) {
			v[1] = AS_NUMBER(field);
		}

		bool result = ImGui::DragInt2(text, v);

		if (result) {
			lit_table_set(state, &inst->fields, CONST_STRING(state, "x"), NUMBER_VALUE(v[0]));
			lit_table_set(state, &inst->fields, CONST_STRING(state, "y"), NUMBER_VALUE(v[1]));
		}

		return BOOL_VALUE(result);
	} else {
		if (!IS_ARRAY(value)) {
			lit_runtime_error_exiting(vm, "Expected array as argument #2");
		}

		LitValues* array = &AS_ARRAY(value)->values;
		int v[2];

		for (uint i = 0; i < fmin(2, array->count); i++) {
			LitValue vl = array->values[i];

			if (IS_NUMBER(vl)) {
				v[i] = AS_NUMBER(vl);
			}
		}

		bool result = ImGui::DragInt2(text, v);

		if (result) {
			for (uint i = 0; i < fmax(2, array->count); i++) {
				array->values[i] = NUMBER_VALUE(v[i]);
			}
		}

		return BOOL_VALUE(result);
	}
}

LIT_METHOD(ui_input_float2) {
	LIT_ENSURE_ARGS(2)

	const char* text = LIT_CHECK_STRING(0);
	LitValue value = args[1];

	if (IS_INSTANCE(value)) {
		LitInstance* inst = AS_INSTANCE(value);
		LitState* state = vm->state;
		LitValue field;

		float v[2];

		if (lit_table_get(&inst->fields, CONST_STRING(state, "x"), &field) && IS_NUMBER(field)) {
			v[0] = AS_NUMBER(field);
		}

		if (lit_table_get(&inst->fields, CONST_STRING(state, "y"), &field) && IS_NUMBER(field)) {
			v[1] = AS_NUMBER(field);
		}

		bool result = ImGui::DragFloat2(text, v);

		if (result) {
			lit_table_set(state, &inst->fields, CONST_STRING(state, "x"), NUMBER_VALUE(v[0]));
			lit_table_set(state, &inst->fields, CONST_STRING(state, "y"), NUMBER_VALUE(v[1]));
		}

		return BOOL_VALUE(result);
	} else {
		if (!IS_ARRAY(value)) {
			lit_runtime_error_exiting(vm, "Expected array as argument #2");
		}

		LitValues* array = &AS_ARRAY(value)->values;
		float v[2];

		for (uint i = 0; i < fmin(2, array->count); i++) {
			LitValue vl = array->values[i];

			if (IS_NUMBER(vl)) {
				v[i] = AS_NUMBER(vl);
			}
		}

		bool result = ImGui::DragFloat2(text, v);

		if (result) {
			for (uint i = 0; i < fmax(2, array->count); i++) {
				array->values[i] = NUMBER_VALUE(v[i]);
			}
		}

		return BOOL_VALUE(result);
	}
}

LIT_METHOD(ui_color_edit) {
	const char* text = LIT_CHECK_STRING(0);
	lit_ensure_object_type(vm, args[1], OBJECT_ARRAY, "Expected a number referenced");

	LitValues* array = &AS_ARRAY(args[1])->values;
	bool result;

	if (array->count > 3) {
		float v[4];

		for (uint i = 0; i < fmin(4, array->count); i++) {
			LitValue value = array->values[i];

			if (IS_NUMBER(value)) {
				v[i] = AS_NUMBER(value) / 255.0f;
			}
		}

		result = ImGui::ColorEdit4(text, v);

		if (result) {
			for (uint i = 0; i < fmax(4, array->count); i++) {
				array->values[i] = NUMBER_VALUE(v[i] * 255);
			}
		}
	} else {
		float v[3];

		for (uint i = 0; i < fmin(3, array->count); i++) {
			LitValue value = array->values[i];

			if (IS_NUMBER(value)) {
				v[i] = AS_NUMBER(value) / 255.0f;
			}
		}

		result = ImGui::ColorEdit3(text, v);

		if (result) {
			for (uint i = 0; i < fmax(3, array->count); i++) {
				array->values[i] = NUMBER_VALUE(v[i] * 255);
			}
		}
	}

	return BOOL_VALUE(result);
}

LIT_METHOD(ui_separator) {
	ImGui::Separator();
	return NULL_VALUE;
}

LIT_METHOD(ui_same_line) {
	ImGui::SameLine();
	return NULL_VALUE;
}

void tsab_ui_bind_api(LitState* state) {
	LIT_BEGIN_CLASS("ImGui")
		LIT_BIND_STATIC_METHOD("newFrame", ui_new_frame)
		LIT_BIND_STATIC_METHOD("render", ui_render)

		LIT_BIND_STATIC_METHOD("setNextWindowPosition", ui_set_next_window_position)
		LIT_BIND_STATIC_METHOD("setNextWindowSize", ui_set_next_window_size)

		LIT_BIND_STATIC_METHOD("begin", ui_begin)
		LIT_BIND_STATIC_METHOD("end", ui_end)
		LIT_BIND_STATIC_METHOD("treeNode", ui_tree_node)
		LIT_BIND_STATIC_METHOD("treePop", ui_tree_pop)

		LIT_BIND_STATIC_METHOD("beginPopup", ui_begin_popup)
		LIT_BIND_STATIC_METHOD("endPopup", ui_end_popup)
		LIT_BIND_STATIC_METHOD("openPopup", ui_open_popup)
		LIT_BIND_STATIC_METHOD("closePopup", ui_close_popup)

		LIT_BIND_STATIC_METHOD("begin", ui_begin)
		LIT_BIND_STATIC_METHOD("end", ui_end)

		LIT_BIND_STATIC_METHOD("text", ui_text)
		LIT_BIND_STATIC_METHOD("bulletText", ui_bullet_text)
		LIT_BIND_STATIC_METHOD("button", ui_button)
		LIT_BIND_STATIC_METHOD("checkbox", ui_checkbox)

		LIT_BIND_STATIC_METHOD("inputInt", ui_input_int)
		LIT_BIND_STATIC_METHOD("inputFloat", ui_input_float)
		LIT_BIND_STATIC_METHOD("dragInt", ui_drag_int)
		LIT_BIND_STATIC_METHOD("dragFloat", ui_drag_float)

		LIT_BIND_STATIC_METHOD("inputInt2", ui_input_int2)
		LIT_BIND_STATIC_METHOD("inputFloat2", ui_input_float2)

		LIT_BIND_STATIC_METHOD("colorEdit", ui_color_edit)

		LIT_BIND_STATIC_METHOD("separator", ui_separator)
		LIT_BIND_STATIC_METHOD("sameLine", ui_same_line)
	LIT_END_CLASS()
}