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

LIT_METHOD(ui_begin) {
	ImGui::SetNextWindowBgAlpha(0.8f);

	const char* name = LIT_GET_STRING(0, "Easter Egg");
	ImGui::Begin(name);

	return NULL_VALUE;
}

LIT_METHOD(ui_end) {
	ImGui::End();
	return NULL_VALUE;
}

LIT_METHOD(ui_text) {
	const char* text = LIT_CHECK_STRING(0);
	ImGui::Text(text);

	return NULL_VALUE;
}


void tsab_ui_bind_api(LitState* state) {
	LIT_BEGIN_CLASS("ImGui")
		LIT_BIND_STATIC_METHOD("newFrame", ui_new_frame)
		LIT_BIND_STATIC_METHOD("render", ui_render)

		LIT_BIND_STATIC_METHOD("begin", ui_begin)
		LIT_BIND_STATIC_METHOD("end", ui_end)
		LIT_BIND_STATIC_METHOD("text", ui_text)
	LIT_END_CLASS()
}