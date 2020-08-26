#include <tsab/tsab_shaders.hpp>

#include <vector>
#include <iostream>

static int active_shader = -1;

static std::vector<Uint32> shaders_separate;
static std::vector<GPU_ShaderBlock> shader_blocks;
static std::vector<Uint32> shaders;
static bool textured_state = false;

static const char *default_vert =
	"attribute vec3 gpu_Vertex;\n"
	"attribute vec2 gpu_TexCoord;\n"
	"attribute vec4 gpu_Color;\n"
	"uniform mat4 gpu_ModelViewProjectionMatrix;\n"
	"\n"
	"varying vec4 color;\n"
	"varying vec2 texCoord;\n"
	"\n"
	"void main() {\n"
	"\tcolor = gpu_Color;\n"
	"\ttexCoord = vec2(gpu_TexCoord);\n"
	"\tgl_Position = gpu_ModelViewProjectionMatrix * vec4(gpu_Vertex, 1.0);\n"
	"}";

void tsab_shaders_quit() {
	for (int i = 0; i < shaders_separate.size(); i++) {
		GPU_FreeShader(shaders_separate[i]);
	}

	for (int i = 0; i < shaders.size(); i++) {
		GPU_FreeShaderProgram(shaders[i]);
	}
}

int tsab_shaders_get_active() {
	return active_shader;
}

Uint32 tsab_shaders_get_active_shader() {
	return shaders[active_shader];
}

/*
 * Lit-side api
 */

LIT_METHOD(tsab_shader_constructor) {
	const char *name = LIT_CHECK_STRING(0);
	bool compile = LIT_GET_BOOL(1, false);

	Uint32 v = GPU_CompileShader(GPU_VERTEX_SHADER, default_vert);

	if (!v) {
		GPU_LogError("Failed to load vertex shader: %s\n", GPU_GetShaderMessage());
	}

	shaders_separate.push_back(v);
	Uint32 f;

	if (compile) {
		f = GPU_CompileShader(GPU_FRAGMENT_SHADER, name);
	} else {
		f = GPU_LoadShader(GPU_FRAGMENT_SHADER, name);
	}

	if (!f) {
		GPU_LogError("Failed to load fragment shader: %s\n", GPU_GetShaderMessage());
	}

	shaders_separate.push_back(f);
	Uint32 p = GPU_LinkShaders(v, f);

	if (!p) {
		GPU_LogError("Failed to link shader program: %s\n", GPU_GetShaderMessage());
	}

	shader_blocks.push_back(GPU_LoadShaderBlock(p, "gpu_Vertex", "gpu_TexCoord", "gpu_Color", "gpu_ModelViewProjectionMatrix"));
	shaders.push_back(p);

	LIT_SET_FIELD("id", shaders.size() - 1);
	return instance;
}

void tsab_shaders_enable(int id) {
	if (id >= -1 && shaders.size() <= id) {
		return;
	}

	active_shader = id;
	GPU_ActivateShaderProgram(shaders[id], &shader_blocks[id]);
}

void tsab_shaders_disable() {
	active_shader = -1;
	GPU_DeactivateShaderProgram();
}

void tsab_shaders_set_textured(bool textured) {
	if (textured_state != textured && active_shader > -1) {
		GPU_SetUniformi(GPU_GetUniformLocation(tsab_shaders_get_active_shader(), "textured"), textured);
		textured_state = textured;
	}
}

LIT_METHOD(tsab_shader_set_float) {
	Uint32 p = (Uint32) AS_NUMBER(LIT_GET_FIELD("id"));

	const char *name = LIT_CHECK_STRING(0);
	float value = (float) LIT_CHECK_NUMBER(1);

	GPU_SetUniformf(GPU_GetUniformLocation(shaders[p], name), value);

	return NULL_VALUE;
}

LIT_METHOD(tsab_shader_set_int) {
	Uint32 p = (Uint32) AS_NUMBER(LIT_GET_FIELD("id"));

	const char *name = LIT_CHECK_STRING(0);
	int value = (int) LIT_CHECK_NUMBER(1);

	GPU_SetUniformi(GPU_GetUniformLocation(shaders[p], name), value);

	return NULL_VALUE;
}

LIT_METHOD(tsab_shader_set_vec2) {
	Uint32 p = (Uint32) AS_NUMBER(LIT_GET_FIELD("id"));

	const char *name = LIT_CHECK_STRING(0);
	float r = (float) LIT_CHECK_NUMBER(1);
	float g = (float) LIT_CHECK_NUMBER(2);

	float values[] = { r, g };
	GPU_SetUniformfv(GPU_GetUniformLocation(shaders[p], name), 2, 1, (float *) values);

	return NULL_VALUE;
}

LIT_METHOD(tsab_shader_set_vec3) {
	Uint32 p = (Uint32) AS_NUMBER(LIT_GET_FIELD("id"));

	const char *name = LIT_CHECK_STRING(0);
	float r = (float) LIT_CHECK_NUMBER(1);
	float g = (float) LIT_CHECK_NUMBER(2);
	float b = (float) LIT_CHECK_NUMBER(3);

	float values[] = { r, g, b };
	GPU_SetUniformfv(GPU_GetUniformLocation(shaders[p], name), 3, 1, (float *) values);

	return NULL_VALUE;
}

LIT_METHOD(tsab_shader_set_vec4) {
	Uint32 p = (Uint32) AS_NUMBER(LIT_GET_FIELD("id"));

	const char *name = LIT_CHECK_STRING(0);
	float r = (float) LIT_CHECK_NUMBER(1);
	float g = (float) LIT_CHECK_NUMBER(2);
	float b = (float) LIT_CHECK_NUMBER(3);
	float a = (float) LIT_CHECK_NUMBER(4);

	float values[] = { r, g, b, a };
	GPU_SetUniformfv(GPU_GetUniformLocation(shaders[p], name), 4, 1, (float *) values);

	return NULL_VALUE;
}

void tsab_shaders_bind_api(LitState* state) {
	LIT_BEGIN_CLASS("Shader")
		LIT_BIND_CONSTRUCTOR(tsab_shader_constructor)

		LIT_BIND_METHOD("setFloat", tsab_shader_set_float)
		LIT_BIND_METHOD("setInt", tsab_shader_set_int)
		LIT_BIND_METHOD("setVec2", tsab_shader_set_vec2)
		LIT_BIND_METHOD("setVec3", tsab_shader_set_vec3)
		LIT_BIND_METHOD("setVec4", tsab_shader_set_vec4)
	LIT_END_CLASS()
}