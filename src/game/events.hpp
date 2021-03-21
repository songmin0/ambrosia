#pragma once
#include "stats_component.hpp"
#include "ai/tree_components.hpp"

#include "entities/tiny_ecs.hpp"
#include "skills/skill_component.hpp"

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
	ProjectileSkillParams skillParams;

	// Callback to be executed at end of projectile trajectory
	std::function<void()> callback;
};

struct HitEvent
{
	ECS::Entity instigator;
	ECS::Entity target;
	float damage;
};

struct ImpulseEvent
{
	ECS::Entity entity;
	vec2 impulse;
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

struct HealEvent
{
	ECS::Entity entity;
	float amount;
};

struct StartMobTurnEvent { };

struct StartMobMoveEvent
{
	// Moving mob
	ECS::Entity entity;
	// Type of movement
	MovementType movement;
};

struct StartMobSkillEvent
{
	// Active mob
	ECS::Entity entity;
	// Target player or mob
	bool targetIsPlayer;
};

struct FinishedMovementEvent
{
	ECS::Entity entity;
};

struct FinishedSkillEvent
{
	ECS::Entity entity;
};

struct PlaySoundEffectEvent
{
	SoundEffect effect = SoundEffect::NONE;
};

struct StartFXEvent
{
	ECS::Entity entity;
	FXType fxType = FXType::NONE;
};

struct StopFXEvent
{
	ECS::Entity entity;
	FXType fxType = FXType::NONE;
};

struct LoadLevelEvent {};
struct StartTutorialEvent {};
struct AdvanceTutorialEvent {};
struct EndTutorialEvent {};
struct ShowHelpEvent {};
struct HideHelpEvent {};
