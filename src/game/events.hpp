#pragma once
#include "stats_component.hpp"

#include "entities/tiny_ecs.hpp"
#include "skills/skill_component.hpp"

using namespace glm;

struct RawMouseClickEvent
{
	vec2 mousePos;
};

struct MouseClickEvent
{
	vec2 mousePos;
};

struct LaunchEvent
{
	ECS::Entity instigator;
	vec2 targetPosition;
	float damage;
	CollisionGroup collisionMask;
};

struct LaunchBulletEvent : public LaunchEvent
{};

struct LaunchBoneEvent : public LaunchEvent
{};

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

struct FinishedMovementEvent
{
	ECS::Entity entity;
};

struct FinishedSkillEvent
{
	ECS::Entity entity;
};
