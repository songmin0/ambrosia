#pragma once
#include "game/common.hpp"
#include "entities/tiny_ecs.hpp"

#include <set>

enum class ProjectileType
{
	BULLET,
	BONE,
	EGG_SHELL
};

enum class Phase
{
	INIT,		// Not launched yet
	PHASE1,	// Travelling toward the target
	PHASE2,	// Travelling back to the instigator (for boomerang)
	END			// Projectile should be removed
};

enum class Trajectory
{
	LINEAR,
	BOOMERANG
};

struct ProjectileParams
{
	ProjectileParams();

	static ProjectileParams create(ProjectileType type, float damage);

	std::string spritePath;
	vec2 spriteScale;
	vec2 launchOffset;
	float launchSpeed;
	float rotationSpeed;
	float damage;
	Trajectory trajectory;
};

struct ProjectileComponent
{
	ProjectileComponent();

	inline void collideWith(ECS::Entity e) {ignoredEntities.insert(e.id);}
	inline bool canCollideWith(ECS::Entity e) const {return ignoredEntities.count(e.id) == 0;}

	ECS::Entity instigator;
	vec2 sourcePosition;
	vec2 targetPosition;
	ProjectileParams params;
	float timeSinceLaunch;
	Phase phase;

	std::set<int> ignoredEntities;

	// This callback is executed when the projectile is finished. When using a linear trajectory, this happens when the
	// projectile reaches the target position. When using a boomerang trajectory, this happens when the projectile gets
	// back to the instigator.
	std::function<void()> callback;
};
