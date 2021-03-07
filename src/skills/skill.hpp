#pragma once

#include "entity_provider.hpp"
#include "entity_filter.hpp"
#include "skill_params.hpp"

#include "game/common.hpp"
#include "game/stats_component.hpp"
#include "entities/tiny_ecs.hpp"
#include "physics/projectile.hpp"

#include <memory>

///////////////////////////////////////////////////////////////////////////////
// Skill
// 	- abstract class
///////////////////////////////////////////////////////////////////////////////

class Skill
{
public:
	Skill(SkillParams params);
	~Skill() = default;

	void performAnimation();
	void performSkill(vec2 target);

	inline float getDelay() const {return params.delay;}
	inline ECS::Entity getInstigator() const {return params.instigator;}
	inline SoundEffect getSoundEffect() const {return params.soundEffect;}

private:
	virtual void performSkillInternal() = 0;

protected:
	SkillParams params;
};

///////////////////////////////////////////////////////////////////////////////
// AreaOfEffectSkill
// 	- abstract class
// 	- collects targets using an EntityProvider
//	- filters those targets using EntityFilters
//	- applies the `process` function to each target
//
//	- subclasses must set up their EntityProvider and EntityFilters in their constructor
//	- subclasses must implement the `process` function which contains the actual logic for the skill
///////////////////////////////////////////////////////////////////////////////

class AreaOfEffectSkill : public Skill
{
public:
	AreaOfEffectSkill(SkillParams params);
	~AreaOfEffectSkill() = default;

private:
	void performSkillInternal() override;

	std::vector<ECS::Entity> getEntities();
	virtual void process(ECS::Entity entity) = 0;

protected:
	std::unique_ptr<EntityProvider> entityProvider;
	std::vector<std::unique_ptr<EntityFilter>> entityFilters;
};

///////////////////////////////////////////////////////////////////////////////
// ProjectileSkill
//	- does not use EntityProviders and EntityFilters like AreaOfEffectSkill does because we don't know where the
// 		projectile is going to go at this point. That's up to the ProjectileSystem. Instead, ProjectileSkill just
// 		launches a projectile, and the ProjectileSystem will take care of the rest.
///////////////////////////////////////////////////////////////////////////////

class ProjectileSkill : public Skill
{
public:
	ProjectileSkill(SkillParams params, ProjectileType projectileType);
	~ProjectileSkill() = default;

	void performSkillInternal() override;

private:
	ProjectileType projectileType;
};




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// CUSTOM AREA OF EFFECT SKILLS
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// MeleeSkill
//	- collects targets within a certain radius around the instigator
//	- sends a HitEvent to each target
//	- you can choose whether the skill only hits the closest target or hits all targets within range
//	- see `skill_params.hpp` for more options and see `raoul.cpp` for usage
///////////////////////////////////////////////////////////////////////////////

class MeleeSkill : public AreaOfEffectSkill
{
public:
	MeleeSkill(SkillParams params);
	~MeleeSkill() = default;

private:
	void process(ECS::Entity entity) override;
};

///////////////////////////////////////////////////////////////////////////////
// BuffSkill
//	- collects targets in one of two ways:
//			- within a certain radius around the instigator (see BuffProximitySkill below)
//			- within a certain radius around the mouse click position (see BuffMouseClickSkill below)
//	- sends a BuffEvent to each target
///////////////////////////////////////////////////////////////////////////////

class BuffSkill : public AreaOfEffectSkill
{
public:
	BuffSkill(SkillParams params, StatModifier statModifier);
	~BuffSkill() = default;

private:
	void process(ECS::Entity entity) override;

protected:
	StatModifier statModifier;
};

///////////////////////////////////////////////////////////////////////////////
// BuffProximitySkill
//	- see notes in the BuffSkill section above
///////////////////////////////////////////////////////////////////////////////

class BuffProximitySkill : public BuffSkill
{
public:
	BuffProximitySkill(SkillParams params, StatModifier statModifier);
	~BuffProximitySkill() = default;
};

///////////////////////////////////////////////////////////////////////////////
// BuffMouseClickSkill
//	- see notes in the BuffSkill section above
///////////////////////////////////////////////////////////////////////////////

class BuffMouseClickSkill : public BuffSkill
{
public:
	BuffMouseClickSkill(SkillParams params, StatModifier statModifier);
	~BuffMouseClickSkill() = default;
};
