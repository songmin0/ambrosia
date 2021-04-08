#pragma once

#include "entity_provider.hpp"
#include "entity_filter.hpp"
#include "entity_handler.hpp"
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
	Skill(std::shared_ptr<SkillParams> params);
	virtual ~Skill() = default;

	void performSkill(vec2 target);

	float getDelay() const;
	AnimationType getAnimationType() const;
	virtual float getRange() = 0;
	//Using this in the RangeIndicator to find out if the skill is centered around the character or the mouse.
	std::shared_ptr<SkillParams> getParams();

private:
	virtual void performSkillInternal() = 0;

protected:
	std::shared_ptr<SkillParams> params;
};

///////////////////////////////////////////////////////////////////////////////
// AreaOfEffectSkill
// 	- collects targets using an EntityProvider
//	- filters those targets using EntityFilters
//	- applies the skill to each target using an EntityHandler
///////////////////////////////////////////////////////////////////////////////

class AreaOfEffectSkill : public Skill
{
public:
	AreaOfEffectSkill(std::shared_ptr<AoESkillParams> params);
	~AreaOfEffectSkill() override = default;

	float getRange() override;

private:
	void performSkillInternal() override;
};

///////////////////////////////////////////////////////////////////////////////
// ProjectileSkill
//	- uses EntityFilters to filter targets that the projectile encounters along
//		its trajectory
//	- applies the skill to each target using an EntityHandler
///////////////////////////////////////////////////////////////////////////////

class ProjectileSkill : public Skill
{
public:
	ProjectileSkill(std::shared_ptr<ProjectileSkillParams> params);
	~ProjectileSkill() override = default;

	float getRange() override;

private:
	void performSkillInternal() override;
};
