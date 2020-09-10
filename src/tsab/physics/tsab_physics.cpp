#include <tsab/physics/tsab_physics.hpp>
#include <tsab/physics/tsab_debug_view.hpp>
#include <tsab/tsab.hpp>

#include <box2d/box2d.h>
#include <SDL2/SDL.h>

#include <vector>

static b2World* world;
static DebugView debug;

static b2Body** extract_body_data_from_instance(LitState* state, LitInstance* instance) {
	LitValue data;

	if (!lit_table_get(&instance->fields, CONST_STRING(state, "_data"), &data)) {
		return NULL;
	}

	return (b2Body**) &AS_USERDATA(data)->data;
}

static void destroy_world(LitState* state) {
	if (world != nullptr) {
		b2Body* body = world->GetBodyList();

		while (body != nullptr) {
			*extract_body_data_from_instance(state, (LitInstance*) body->GetUserData()) = nullptr;
			body = body->GetNext();
		}

		b2Joint* joint = world->GetJointList();

		while (joint != nullptr) {
			b2Joint* next = joint->GetNext();
			world->DestroyJoint(joint);
			joint = next;
		}

		delete world;
		world = nullptr;
	}
}

void tsab_physics_quit(LitState* state) {
	destroy_world(state);
}

/*
 * Body class
 */

static b2Body* extract_body_data(LitState* state, LitValue instance) {
	LitValue data;

	if (!lit_table_get(&AS_INSTANCE(instance)->fields, CONST_STRING(state, "_data"), &data)) {
		return NULL;
	}

	b2Body* body = (b2Body*) AS_USERDATA(data)->data;

	if (body == nullptr) {
		lit_runtime_error_exiting(state->vm, "Attempt to access invalid body");
	}

	return body;
}

void cleanup_body(LitState* state, LitUserdata* data) {
	if (world != nullptr) {
		b2Body* body = (b2Body*) data->data;

		if (body != nullptr) {
			body->SetUserData(nullptr);
			world->DestroyBody(body);
		}
	}
}

LIT_METHOD(body_constructor) {
	if (world == nullptr) {
		lit_runtime_error_exiting(vm, "Attempted to create a body in non-existing world!");
	}

	b2BodyDef def;
	def.linearDamping = 0.1f;
	def.angularDamping = 0.1f;

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

	b2Body* body = world->CreateBody(&def);

	LitUserdata* userdata = lit_create_userdata(vm->state, 0);
	userdata->cleanup_fn = cleanup_body;
	userdata->data = body;
	lit_table_set(vm->state, &AS_INSTANCE(instance)->fields, CONST_STRING(vm->state, "_data"), OBJECT_VALUE(userdata));

	body->SetUserData((void*) AS_INSTANCE(instance));

	b2FixtureDef fixture;
	fixture.density = 1;
	fixture.restitution = 0.6;

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
		float x, y, r;

		if (arg_count < 4 || (arg_count < 5 && IS_BOOL(args[3]))) {
			x = 0;
			y = 0;
			r = LIT_CHECK_NUMBER(2);
			fixture.isSensor = LIT_GET_BOOL(3, false);
		} else {
			x = LIT_CHECK_NUMBER(2);
			y = LIT_CHECK_NUMBER(3);
			r = LIT_CHECK_NUMBER(4);
			fixture.isSensor = LIT_GET_BOOL(5, false);
		}

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

LIT_METHOD(body_apply_force) {
	b2Body* body = extract_body_data(vm->state, instance);

	float x = LIT_CHECK_NUMBER(0);
	float y = LIT_CHECK_NUMBER(1);

	if (arg_count == 2) {
		body->ApplyForceToCenter(b2Vec2(x, y), true);
	} else {
		float cx = LIT_CHECK_NUMBER(2);
		float cy = LIT_CHECK_NUMBER(3);

		body->ApplyForce(b2Vec2(x, y), b2Vec2(cx, cy), true);
	}

	return NULL_VALUE;
}

LIT_METHOD(body_apply_impulse) {
	b2Body* body = extract_body_data(vm->state, instance);

	float x = LIT_CHECK_NUMBER(0);
	float y = LIT_CHECK_NUMBER(1);

	if (arg_count == 2) {
		body->ApplyLinearImpulseToCenter(b2Vec2(x, y), true);
	} else {
		float cx = LIT_CHECK_NUMBER(2);
		float cy = LIT_CHECK_NUMBER(3);

		body->ApplyLinearImpulse(b2Vec2(x, y), b2Vec2(cx, cy), true);
	}

	return NULL_VALUE;
}

LIT_METHOD(body_bounciness) {
	b2Body* body = extract_body_data(vm->state, instance);
	float bounciness = LIT_CHECK_NUMBER(0);
	b2Fixture* fixture = body->GetFixtureList();

	while (fixture != nullptr) {
		fixture->SetRestitution(bounciness);
		fixture = fixture->GetNext();
	}

	return NULL_VALUE;
}

LIT_METHOD(body_density) {
	b2Body* body = extract_body_data(vm->state, instance);
	float density = LIT_CHECK_NUMBER(0);
	b2Fixture* fixture = body->GetFixtureList();

	while (fixture != nullptr) {
		fixture->SetDensity(density);
		fixture = fixture->GetNext();
	}

	return NULL_VALUE;
}

LIT_METHOD(body_friction) {
	b2Body* body = extract_body_data(vm->state, instance);
	float friction = LIT_CHECK_NUMBER(0);
	b2Fixture* fixture = body->GetFixtureList();

	while (fixture != nullptr) {
		fixture->SetFriction(friction);
		fixture = fixture->GetNext();
	}

	return NULL_VALUE;
}

LIT_METHOD(body_sensor) {
	b2Body* body = extract_body_data(vm->state, instance);
	bool sensor = LIT_CHECK_BOOL(0);
	b2Fixture* fixture = body->GetFixtureList();

	while (fixture != nullptr) {
		fixture->SetSensor(sensor);
		fixture = fixture->GetNext();
	}

	return NULL_VALUE;
}

LIT_METHOD(body_bullet) {
	b2Body* body = extract_body_data(vm->state, instance);

	if (arg_count == 0) {
		return BOOL_VALUE(body->IsBullet());
	}

	bool bullet = LIT_CHECK_BOOL(0);
	body->SetBullet(bullet);
	return args[0];
}

LIT_METHOD(body_linear_damping) {
	b2Body* body = extract_body_data(vm->state, instance);

	if (arg_count == 0) {
		return NUMBER_VALUE(body->GetLinearDamping());
	}

	float damping = LIT_CHECK_NUMBER(0);
	body->SetLinearDamping(damping);
	return args[0];
}

LIT_METHOD(body_angular_damping) {
	b2Body* body = extract_body_data(vm->state, instance);

	if (arg_count == 0) {
		return NUMBER_VALUE(body->GetAngularDamping());
	}

	float damping = LIT_CHECK_NUMBER(0);
	body->SetAngularDamping(damping);
	return args[0];
}

LIT_METHOD(body_angular_velocity) {
	b2Body* body = extract_body_data(vm->state, instance);

	if (arg_count == 0) {
		return NUMBER_VALUE(body->GetAngularVelocity());
	}

	float vel = LIT_CHECK_NUMBER(0);
	body->SetAngularVelocity(vel);
	return args[0];
}

LIT_METHOD(body_velocity_x) {
	b2Body* body = extract_body_data(vm->state, instance);
	b2Vec2 velocity = body->GetLinearVelocity();

	if (arg_count == 0) {
		return NUMBER_VALUE(velocity.x);
	}

	velocity.x = LIT_CHECK_NUMBER(0);
	body->SetLinearVelocity(velocity);
	return args[0];
}

LIT_METHOD(body_velocity_y) {
	b2Body* body = extract_body_data(vm->state, instance);
	b2Vec2 velocity = body->GetLinearVelocity();

	if (arg_count == 0) {
		return NUMBER_VALUE(velocity.y);
	}

	velocity.y = LIT_CHECK_NUMBER(0);
	body->SetLinearVelocity(velocity);
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

	debug.SetFlags(b2Draw::e_shapeBit | b2Draw::e_jointBit);

	return NULL_VALUE;
}

LIT_METHOD(physics_destroy_world) {
	destroy_world(vm->state);
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

/*LIT_METHOD(physics_join) {
	if (world == nullptr) {
		return NULL_VALUE;
	}

	const char* type = LIT_CHECK_STRING(0);
	LitInstance* a = LIT_CHECK_INSTANCE(1);
	LitInstance* b = LIT_CHECK_INSTANCE(2);

	b2Body* body_a = *extract_body_data_from_instance(vm->state, a);
	b2Body* body_b = *extract_body_data_from_instance(vm->state, b);

	b2DistanceJointDef def;

	def.bodyA = body_a;
	def.bodyB = body_b;
	def.length = 128;
	def.maxLength = 164;
	def.minLength = 128;
	def.stiffness = 1;
	def.localAnchorA = body_a->GetLocalCenter();
	def.localAnchorB = body_b->GetLocalCenter();
	def.collideConnected = false;

	world->CreateJoint(&def);

	return NULL_VALUE;
}*/

void tsab_physics_bind_api(LitState* state) {
	LIT_BEGIN_CLASS("Body")
		LIT_BIND_CONSTRUCTOR(body_constructor)

		LIT_BIND_FIELD("x", body_x, body_x)
		LIT_BIND_FIELD("y", body_y, body_y)
		LIT_BIND_FIELD("angle", body_angle, body_angle)
		LIT_BIND_FIELD("bullet", body_bullet, body_bullet)
		LIT_BIND_FIELD("linearDamping", body_linear_damping, body_linear_damping)
		LIT_BIND_FIELD("angularDamping", body_angular_damping, body_angular_damping)
		LIT_BIND_FIELD("angularVelocity", body_angular_velocity, body_angular_velocity)
		LIT_BIND_FIELD("velocityX", body_velocity_x, body_velocity_x)
		LIT_BIND_FIELD("velocityY", body_velocity_y, body_velocity_y)

		LIT_BIND_SETTER("bounciness", body_bounciness)
		LIT_BIND_SETTER("friction", body_friction)
		LIT_BIND_SETTER("density", body_density)
		LIT_BIND_SETTER("sensor", body_sensor)

		LIT_BIND_METHOD("applyForce", body_apply_force)
		LIT_BIND_METHOD("applyImpulse", body_apply_impulse)
	LIT_END_CLASS()

	LIT_BEGIN_CLASS("Physics")
		LIT_BIND_STATIC_METHOD("newWorld", physics_new_world)
		LIT_BIND_STATIC_METHOD("destroyWorld", physics_destroy_world)
		LIT_BIND_STATIC_METHOD("update", physics_update)
		LIT_BIND_STATIC_METHOD("render", physics_render)
		// LIT_BIND_STATIC_METHOD("join", physics_join)
	LIT_END_CLASS()
}