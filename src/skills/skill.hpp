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

private:
	void performSkillInternal() override;
};
