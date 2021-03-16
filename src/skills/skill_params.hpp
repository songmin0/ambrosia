#pragma once
#include "entity_provider.hpp"
#include "entity_filter.hpp"
#include "entity_handler.hpp"

#include "game/common.hpp"
#include "entities/tiny_ecs.hpp"
#include "animation/animation_components.hpp"
#include "physics/projectile.hpp"

struct SkillParams
{
	SkillParams()
		: soundEffect(SoundEffect::NONE)
		, animationType(AnimationType::IDLE)
		, delay(0.f)
		, targetPosition(0.f)
	{}
	virtual ~SkillParams() = default;

	ECS::Entity instigator;
	SoundEffect soundEffect;
	AnimationType animationType;
	float delay;	// A delay to allow us to sync up a skill with its animation

	std::vector<std::shared_ptr<EntityFilter>> entityFilters;
	std::shared_ptr<EntityHandler> entityHandler;

	vec2 targetPosition; // To be filled when the skill is executed
};

struct AoESkillParams : public SkillParams
{
	AoESkillParams() = default;
	~AoESkillParams() override = default;

	std::shared_ptr<EntityProvider> entityProvider;
};

struct ProjectileSkillParams : public SkillParams
{
	ProjectileSkillParams()
		: projectileType(ProjectileType::INVALID)
	{}
	~ProjectileSkillParams() override = default;

	ProjectileType projectileType;
};
