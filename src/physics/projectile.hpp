#pragma once
#include "game/common.hpp"
#include "entities/tiny_ecs.hpp"
#include "skills/entity_filter.hpp"
#include "skills/entity_handler.hpp"

#include <set>
#include <functional>

enum class ProjectileType
{
	INVALID,
	BLUEBERRY,
	BONE,
	EGG_SHELL,
	HEAL_ORB,
	DAMAGE_ORB,
	PEPPER,
	SALT,
	DRUMSTICK,
	AMBROSIA_ICON
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
	CURVED,
	BOOMERANG
};

struct ProjectileParams
{
	ProjectileParams();

	static ProjectileParams create(ProjectileType type);

	std::string spritePath;
	vec2 spriteScale;
	vec2 launchOffset;
	float launchSpeed;
	float rotationSpeed;
	Trajectory trajectory;
};

struct ProjectileComponent
{
	ProjectileComponent();

	void processCollision(ECS::Entity entity);

public:
	ECS::Entity instigator;
	vec2 sourcePosition;
	vec2 targetPosition;
	ProjectileParams params;
	float timeSinceLaunch;
	Phase phase;

	std::set<int> ignoredEntities;
	std::vector<std::shared_ptr<EntityFilter>> entityFilters;
	std::shared_ptr<EntityHandler> entityHandler;

	// This callback is executed when the projectile is finished. When using a
	// linear trajectory, this happens when the projectile reaches the target
	// position. When using a boomerang trajectory, this happens when the
	// projectile gets back to the instigator.
	std::function<void()> callback;

	// If provided, this will override the `targetPosition` field. Only applies
	// to projectiles using a curved trajectory.
	std::function<vec2()> targetPositionProvider;
};

struct AmbrosiaProjectileComponent {};
