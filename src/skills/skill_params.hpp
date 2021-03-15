#pragma once

#include "animation/animation_components.hpp"
#include "game/common.hpp"
#include "entities/tiny_ecs.hpp"

struct SkillParams
{
	SkillParams()
		: animationType(AnimationType::IDLE)
		, delay(0.f)
		, damage(0.f)
		, range(0.f)
		, angle(0.f)
		, ignoreInstigator(true)
		, collideWithMultipleEntities(false)
		, collidesWith(CollisionGroup::NONE)
		, targetPosition(0.f)
		, soundEffect(SoundEffect::NONE)
	{}

	ECS::Entity instigator;

	AnimationType animationType;
	float delay;	// A delay to allow us to sync up a skill with its animation
	float damage;	// Base damage value. In the StatsSystem, this is combined with the instigator's strength to calculate the actual damage value
	float range;	// See usage in CircularProvider and MouseClickProvider
	float angle;	// See usage in ConicalProvider

	// If true, the skill won't affect the instigator. The `collidesWith` field will keep the instigator safe from their own
	// attacks. This `ignoreInstigator` flag is really just for buffs because in that case, `collidesWith` will be set to the
	// same type as the instigator. By default, this is set to false, so the instigator won't be affected by their own buffs.
	bool ignoreInstigator;

	// If true, all eligible targets will be affected/hit by this skill. If false, only the closest entity will be affected
	bool collideWithMultipleEntities;

	// This controls the types of entities that will be affected/hit by this skill
	CollisionGroup collidesWith;

	vec2 targetPosition; // To be filled when the skill is executed

	SoundEffect soundEffect;
};
