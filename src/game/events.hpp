#pragma once
#include "stats_component.hpp"

#include "entities/tiny_ecs.hpp"
#include "skills/skill_component.hpp"
#include "physics/projectile.hpp"

#include <functional>

using namespace glm;

struct RawMouseClickEvent
{
	vec2 mousePos;
};

struct MouseClickEvent
{
	vec2 mousePos;
};

struct RawMouseHoverEvent
{
	vec2 mousePos;
};

struct LaunchEvent
{
	ProjectileType projectileType;

	ECS::Entity instigator;
	vec2 targetPosition;
	float damage;
	CollisionGroup collisionMask;

	// Callback to be executed at end of projectile trajectory
	std::function<void()> callback;
};

struct HitEvent
{
	ECS::Entity instigator;
	ECS::Entity target;
	float damage;
};

struct PlayerButtonEvent
{
	PlayerType player;
};

struct PlayerChangeEvent
{
	ECS::Entity newActiveEntity;
};

struct SetActiveSkillEvent
{
	ECS::Entity entity;
	SkillType type;
};

struct PerformActiveSkillEvent
{
	ECS::Entity entity;
	vec2 target;
};

struct BuffEvent
{
	ECS::Entity entity;
	StatModifier statModifier;
};

struct StartMobMovementEvent
{
	ECS::Entity entity;
};

struct StartMobSkillEvent
{
	ECS::Entity entity;
};

struct FinishedMovementEvent
{
	ECS::Entity entity;
};

struct FinishedSkillEvent
{
	ECS::Entity entity;
};
