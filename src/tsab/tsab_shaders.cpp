#include <tsab/tsab_shaders.hpp>

#include <vector>
#include <iostream>

static int active_shader = -1;

static std::vector<Uint32> shaders_separate;
static std::vector<GPU_ShaderBlock> shader_blocks;
static std::vector<Uint32> shaders;

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

LIT_METHOD(tsab_shaders_new) {
	Uint32 v = GPU_CompileShader(GPU_VERTEX_SHADER, default_vert);

	if (!v) {
		GPU_LogError("Failed to load vertex shader: %s\n", GPU_GetShaderMessage());
	}

	shaders_separate.push_back(v);

	const char *name = LIT_CHECK_STRING(0);
	bool compile = LIT_GET_BOOL(1, false);
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

	return NUMBER_VALUE(shaders.size() - 1);
}

LIT_METHOD(tsab_shaders_set) {
	if (arg_count == 0) {
		active_shader = -1;
		GPU_DeactivateShaderProgram();

		return NULL_VALUE;
	}

	Uint32 p = (Uint32) LIT_CHECK_NUMBER(0);

	if (p >= -1 && shaders.size() <= p) {
		return 0;
	}

	active_shader = p;
	GPU_ActivateShaderProgram(shaders[p], &shader_blocks[p]);

	return NULL_VALUE;
}

LIT_METHOD(tsab_shaders_setFloat) {
	Uint32 p = (Uint32) LIT_CHECK_NUMBER(0);
	const char *name = LIT_CHECK_STRING(1);
	float value = (float) LIT_CHECK_NUMBER(2);

	int location = GPU_GetUniformLocation(shaders[p], name);

	if (location == -1) {
		std::cout << "Unknown variable '" << name << "'!\n";
	} else {
		GPU_SetUniformf(location, value);
	}

	return NULL_VALUE;
}

void tsab_shaders_bind_api(LitState* state) {
	LIT_BEGIN_CLASS("Shaders")
		LIT_BIND_STATIC_METHOD("newShader", tsab_shaders_new)
		LIT_BIND_STATIC_METHOD("setShader", tsab_shaders_set)
		LIT_BIND_STATIC_METHOD("setFloat", tsab_shaders_setFloat)
	LIT_END_CLASS()

	/*lua_register(L, "tsab_shaders_send_float", tsab_shaders_send_float);
	lua_register(L, "tsab_shaders_send_int", tsab_shaders_send_int);
	lua_register(L, "tsab_shaders_send_vec4", tsab_shaders_send_vec4);
	lua_register(L, "tsab_shaders_send_vec3", tsab_shaders_send_vec3);
	lua_register(L, "tsab_shaders_send_vec2", tsab_shaders_send_vec2);*/
}