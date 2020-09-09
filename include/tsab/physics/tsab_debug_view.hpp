#ifndef TSAB_DEBUG_VIEW_HPP
#define TSAB_DEBUG_VIEW_HPP

#include <box2d/box2d.h>

class DebugView : public b2Draw {
	public:
		void DrawPolygon(const b2Vec2* vertices, int vertexCount, const b2Color& color);
		void DrawSolidPolygon(const b2Vec2* vertices, int vertexCount, const b2Color& color);
		void DrawCircle(const b2Vec2& center, float radius, const b2Color& color);
		void DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color);
		void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color);
		void DrawTransform(const b2Transform& xf);
		void DrawPoint(const b2Vec2& p, float size, const b2Color& color);
};

#endif