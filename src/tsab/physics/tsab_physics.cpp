#include <tsab/physics/tsab_physics.hpp>
#include <tsab/physics/tsab_debug_view.hpp>
#include <tsab/tsab.hpp>

#include <box2d/box2d.h>

static b2World* world;
static DebugView debug;

static void destroy_world() {
	if (world != nullptr) {
		delete world;
		world = nullptr;
	}
}

void tsab_physics_quit() {
	destroy_world();
}

/*
 * Body class
 */

static b2Body* extract_body_data(LitState* state, LitValue instance) {
	LitValue data;

	if (!lit_table_get(&AS_INSTANCE(instance)->fields, CONST_STRING(state, "_data"), &data)) {
		return NULL;
	}

	return (b2Body*) AS_USERDATA(data)->data;
}

void cleanup_body(LitState* state, LitUserdata* data) {
	if (world != nullptr) {
		world->DestroyBody((b2Body*) data->data);
	}
}

LIT_METHOD(body_constructor) {
	if (world == nullptr) {
		lit_runtime_error_exiting(vm, "Attempted to create a body in non-existing world!");
	}

	b2BodyDef def;
	const char* type = AS_CSTRING(args[1]);

	if (memcmp(type, "dynamic", 7) == 0) {
		def.type = b2_dynamicBody;
	} else if (memcmp(type, "static", 6) == 0) {
		def.type = b2_staticBody;
	} else if (memcmp(type, "kinematic", 9) == 0) {
		def.type = b2_kinematicBody;
	} else {
		lit_runtime_error_exiting(vm, "Unknown body type %s", type);
	}

	def.allowSleep = false;
	def.gravityScale = 1;
	b2Body* body = world->CreateBody(&def);

	LitUserdata* userdata = lit_create_userdata(vm->state, 0);
	userdata->cleanup_fn = cleanup_body;
	userdata->data = body;
	lit_table_set(vm->state, &AS_INSTANCE(instance)->fields, CONST_STRING(vm->state, "_data"), OBJECT_VALUE(userdata));

	body->SetUserData((void*) instance);

	b2FixtureDef fixture;
	fixture.density = 1;

	const char* preset = AS_CSTRING(args[0]);

	if (memcmp(preset, "rect", 4) == 0) {
		float x, y, w, h;

		if (arg_count < 5 || (arg_count < 6 && IS_BOOL(args[4]))) {
			x = 0;
			y = 0;
			w = LIT_CHECK_NUMBER(2);
			h = LIT_CHECK_NUMBER(3);
			fixture.isSensor = LIT_GET_BOOL(4, false);
		} else {
			x = LIT_CHECK_NUMBER(2);
			y = LIT_CHECK_NUMBER(3);
			w = LIT_CHECK_NUMBER(4);
			h = LIT_CHECK_NUMBER(5);
			fixture.isSensor = LIT_GET_BOOL(6, false);
		}

		b2Vec2 vertices[4];

		vertices[0].Set(x, y);
		vertices[1].Set(x, y + h);
		vertices[2].Set(x + w, y + h);
		vertices[3].Set(x + w, y);

		b2PolygonShape polygonShape;

		polygonShape.Set(vertices, 4);
		fixture.shape = &polygonShape;
		body->CreateFixture(&fixture);
	} else if (memcmp(preset, "circle", 6) == 0) {
		float x = LIT_CHECK_NUMBER(2);
		float y = LIT_CHECK_NUMBER(3);
		float r = LIT_CHECK_NUMBER(4);
		fixture.isSensor = LIT_GET_BOOL(5, false);

		b2CircleShape circleShape;

		circleShape.m_radius = r;
		circleShape.m_p.x = x + r;
		circleShape.m_p.y = y + r;

		fixture.shape = &circleShape;
		body->CreateFixture(&fixture);
	} else {
		lit_runtime_error_exiting(vm, "Unknown body preset %s", preset);
	}

	return instance;
}

LIT_METHOD(body_x) {
	b2Body* body = extract_body_data(vm->state, instance);
	b2Vec2 pos = body->GetPosition();

	if (arg_count == 0) {
		return NUMBER_VALUE(pos.x);
	}

	pos.x = LIT_CHECK_NUMBER(0);
	body->SetTransform(pos, body->GetAngle());

	return args[0];
}

LIT_METHOD(body_y) {
	b2Body* body = extract_body_data(vm->state, instance);
	b2Vec2 pos = body->GetPosition();

	if (arg_count == 0) {
		return NUMBER_VALUE(pos.y);
	}

	pos.y = LIT_CHECK_NUMBER(0);
	body->SetTransform(pos, body->GetAngle());

	return args[0];
}

LIT_METHOD(body_angle) {
	b2Body* body = extract_body_data(vm->state, instance);

	if (arg_count == 0) {
		return NUMBER_VALUE(body->GetAngle());
	}

	float angle = LIT_CHECK_NUMBER(0);
	body->SetTransform(body->GetPosition(), angle);

	return args[0];
}

/*
 * Physics class
 */

LIT_METHOD(physics_new_world) {
	if (world != nullptr) {
		return NULL_VALUE;
	}

	float gravity_x = LIT_GET_NUMBER(0, 0);
	float gravity_y = LIT_GET_NUMBER(1, -1);
	bool allow_sleep = LIT_GET_BOOL(2, true);

	world = new b2World(b2Vec2(gravity_x, gravity_y));
	world->SetAllowSleeping(allow_sleep);
	world->SetDebugDraw(&debug);

	debug.SetFlags(b2Draw::e_shapeBit);

	return NULL_VALUE;
}

LIT_METHOD(physics_destroy_world) {
	destroy_world();
	return NULL_VALUE;
}

LIT_METHOD(physics_update) {
	if (world != nullptr) {
		float dt = LIT_GET_NUMBER(0, tsab_get_dt());
		world->Step(dt, 8, 3);
	}

	return NULL_VALUE;
}

LIT_METHOD(physics_render) {
	if (world != nullptr) {
		world->DebugDraw();
	}

	return NULL_VALUE;
}

void tsab_physics_bind_api(LitState* state) {
	LIT_BEGIN_CLASS("Body")
		LIT_BIND_CONSTRUCTOR(body_constructor)

		LIT_BIND_FIELD("x", body_x, body_x)
		LIT_BIND_FIELD("y", body_y, body_y)
		LIT_BIND_FIELD("angle", body_angle, body_angle)
	LIT_END_CLASS()

	LIT_BEGIN_CLASS("Physics")
		LIT_BIND_STATIC_METHOD("newWorld", physics_new_world)
		LIT_BIND_STATIC_METHOD("destroyWorld", physics_destroy_world)
		LIT_BIND_STATIC_METHOD("update", physics_update)
		LIT_BIND_STATIC_METHOD("render", physics_render)
	LIT_END_CLASS()
}