#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"

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

	static ProjectileParams createBulletParams();
	static ProjectileParams createBoneParams();

	std::string spritePath;
	vec2 spriteScale;
	vec2 launchOffset;
	float launchSpeed;
	float rotationSpeed;
	float delay;
	Trajectory trajectory;
};

struct ProjectileComponent
{
	ProjectileComponent();

	ECS::Entity instigator;
	vec2 sourcePosition;
	vec2 targetPosition;
	ProjectileParams params;
	float timeSinceLaunch;
	Phase phase;
};
