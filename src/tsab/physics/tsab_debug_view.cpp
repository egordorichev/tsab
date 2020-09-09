#include <tsab/physics/tsab_debug_view.hpp>
#include <tsab/tsab_shaders.hpp>
#include <tsab/tsab_graphics.hpp>

static void update_shader(const b2Color& color) {
	if (tsab_shaders_get_active() > -1) {
		float colors[] = {
			(float) color.r, (float) color.g, (float) color.b, (float) color.a
		};

		GPU_SetUniformfv(GPU_GetUniformLocation(tsab_shaders_get_active_shader(), "color"), 4, 1, (float *) colors);
		GPU_SetUniformf(GPU_GetUniformLocation(tsab_shaders_get_active_shader(), "textured"), 0);
	}
}

void DebugView::DrawPolygon(const b2Vec2* vertices, int vertexCount, const b2Color& color) {
	float ver[vertexCount * 2];

	for (int i = 0; i < vertexCount; i++) {
		ver[i * 2] = vertices[i].x + 0.5;
		ver[i * 2 + 1] = vertices[i].y + 0.5;
	}

	update_shader(color);
	GPU_Polygon(tsab_graphics_get_current_target(), vertexCount, ver, { (Uint8) (color.r * 255), (Uint8) (color.g * 255), (Uint8) (color.b * 255), (Uint8) (color.a * 255) });
}

void DebugView::DrawSolidPolygon(const b2Vec2* vertices, int vertexCount, const b2Color& color) {
	float ver[vertexCount * 2];

	for (int i = 0; i < vertexCount; i++) {
		ver[i * 2] = vertices[i].x + 0.5;
		ver[i * 2 + 1] = vertices[i].y + 0.5;
	}

	if (tsab_shaders_get_active() > -1) {
		float colors[] = {
			(float) color.r, (float) color.g, (float) color.b, (float) color.a
		};

		GPU_SetUniformfv(GPU_GetUniformLocation(tsab_shaders_get_active_shader(), "color"), 4, 1, (float *) colors);
		GPU_SetUniformf(GPU_GetUniformLocation(tsab_shaders_get_active_shader(), "textured"), 0);
	}

	GPU_Polygon(tsab_graphics_get_current_target(), vertexCount, ver, { (Uint8) (color.r * 255), (Uint8) (color.g * 255), (Uint8) (color.b * 255), (Uint8) (color.a * 255) });
}

void DebugView::DrawCircle(const b2Vec2& center, float radius, const b2Color& color) {
	if (tsab_shaders_get_active() > -1) {
		float colors[] = {
			(float) color.r, (float) color.g, (float) color.b, (float) color.a
		};

		GPU_SetUniformfv(GPU_GetUniformLocation(tsab_shaders_get_active_shader(), "color"), 4, 1, (float *) colors);
		GPU_SetUniformf(GPU_GetUniformLocation(tsab_shaders_get_active_shader(), "textured"), 0);
	}

	GPU_Circle(tsab_graphics_get_current_target(), center.x + 0.5, center.y + 0.5, radius, { (Uint8) (color.r * 255), (Uint8) (color.g * 255), (Uint8) (color.b * 255), (Uint8) (color.a * 255) });
}

void DebugView::DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color) {
	if (tsab_shaders_get_active() > -1) {
		float colors[] = {
			(float) color.r, (float) color.g, (float) color.b, (float) color.a
		};

		GPU_SetUniformfv(GPU_GetUniformLocation(tsab_shaders_get_active_shader(), "color"), 4, 1, (float *) colors);
		GPU_SetUniformf(GPU_GetUniformLocation(tsab_shaders_get_active_shader(), "textured"), 0);
	}

	GPU_Circle(tsab_graphics_get_current_target(), center.x + 0.5, center.y + 0.5, radius, { (Uint8) (color.r * 255), (Uint8) (color.g * 255), (Uint8) (color.b * 255), (Uint8) (color.a * 255) });
}

void DebugView::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) {

}

void DebugView::DrawTransform(const b2Transform& xf) {

}

void DebugView::DrawPoint(const b2Vec2& p, float size, const b2Color& color) {
	GPU_Pixel(tsab_graphics_get_current_target(), p.x + 0.5, p.x + 0.5, { (Uint8) (color.r * 255), (Uint8) (color.g * 255), (Uint8) (color.b * 255), (Uint8) (color.a * 255) });
}