#pragma once
#include "game/common.hpp"
#include "game/event_system.hpp"
#include "game/events.hpp"
#include "entities/tiny_ecs.hpp"
#include "maps/path_finding_system.hpp"

const float THRESHOLD = 3.f;
const float DEFAULT_SPEED = 150.f;

struct BoundingBox
{
	inline vec2 center() const {return vec2((left + right) / 2.f, (top + bottom) / 2.f);}
	inline vec2 size() const {return vec2(right - left, bottom - top);}

	float left;
	float right;
	float top;
	float bottom;
};

// A simple physics system that moves rigid bodies and checks for collision
class PhysicsSystem
{
public:
	PhysicsSystem(PathFindingSystem& pfs);
	~PhysicsSystem();

	void step(float elapsed_ms, vec2 window_size_in_game_units);
	void blendMotionData(float alpha);

	static BoundingBox getBoundingBox(ECS::Entity entity, const Motion& motion);

	// Stucture to store collision information
	struct Collision
	{
		// Note, the first object is stored in the ECS container.entities
		ECS::Entity other; // the second object involved in the collision
		Collision(ECS::Entity& other);
	};

private:
	void applyFriction(float& speed, float step_seconds);
	void onImpulseEvent(const ImpulseEvent& event);

	EventListenerInfo impulseEventListener;

	PathFindingSystem& pathFindingSystem;
};
