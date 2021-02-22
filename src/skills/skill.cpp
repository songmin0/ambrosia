#include "skill.hpp"

#include <game/event_system.hpp>
#include <game/events.hpp>

///////////////////////////////////////////////////////////////////////////////
// Skill
///////////////////////////////////////////////////////////////////////////////

Skill::Skill(SkillParams params)
	: params(params)
{}

void Skill::performAnimation()
{
	if (params.instigator.has<AnimationsComponent>())
	{
		auto& anim = params.instigator.get<AnimationsComponent>();
		anim.changeAnimation(params.animationType);
	}
}

///////////////////////////////////////////////////////////////////////////////
// AreaOfEffectSkill
///////////////////////////////////////////////////////////////////////////////

AreaOfEffectSkill::AreaOfEffectSkill(SkillParams params)
	: Skill(params)
{}

void AreaOfEffectSkill::performSkill(vec2 target)
{
	// Update the internal params
	params.sourcePosition = params.instigator.get<Motion>().position;
	params.targetPosition = target;

	// Get the entities that should be affected by this skill
	std::vector<ECS::Entity> entities = getEntities();

	// Apply the skill to each entity
	for (auto entity : entities)
	{
		process(entity);
	}
}

std::vector<ECS::Entity> AreaOfEffectSkill::getEntities()
{
	std::vector<ECS::Entity> entities;

	if (entityProvider)
	{
		entities = entityProvider->getEntities(params);

		if (!entities.empty())
		{
			for (auto& filter : entityFilters)
			{
				entities = filter->process(params, entities);
			}

			if (params.ignoreInstigator)
			{
				InstigatorFilter instigatorFilter;
				entities = instigatorFilter.process(params, entities);
			}

			if (!params.collideWithMultipleEntities)
			{
				entities.resize(1);
			}
		}
	}

	return entities;
}

///////////////////////////////////////////////////////////////////////////////
// ProjectileSkill
///////////////////////////////////////////////////////////////////////////////

ProjectileSkill::ProjectileSkill(SkillParams params)
	: Skill(params)
{

}

void ProjectileSkill::performSkill(vec2 target)
{
	// Update the internal params
	params.sourcePosition = params.instigator.get<Motion>().position;
	params.targetPosition = target;

	process();
}




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// CUSTOM AREA OF EFFECT SKILLS
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// MeleeSkill
///////////////////////////////////////////////////////////////////////////////

MeleeSkill::MeleeSkill(SkillParams params)
	: AreaOfEffectSkill(params)
{
	entityProvider = std::make_unique<CircularProvider>();

	entityFilters.push_back(std::make_unique<CollisionFilter>());
}

void MeleeSkill::process(ECS::Entity entity)
{
	HitEvent hitEvent;
	hitEvent.instigator = params.instigator;
	hitEvent.target = entity;
	hitEvent.damage = params.damage;
	EventSystem<HitEvent>::instance().sendEvent(hitEvent);
}

///////////////////////////////////////////////////////////////////////////////
// BuffSkill
///////////////////////////////////////////////////////////////////////////////

BuffSkill::BuffSkill(SkillParams params, StatModifier statModifier)
	: AreaOfEffectSkill(params)
	, statModifier(statModifier)
{}

void BuffSkill::process(ECS::Entity entity)
{
	BuffEvent buffEvent;
	buffEvent.entity = entity;
	buffEvent.statModifier = statModifier;
	EventSystem<BuffEvent>::instance().sendEvent(buffEvent);
}

///////////////////////////////////////////////////////////////////////////////
// BuffProximitySkill
///////////////////////////////////////////////////////////////////////////////

BuffProximitySkill::BuffProximitySkill(SkillParams params, StatModifier statModifier)
	: BuffSkill(params, statModifier)
{
	entityProvider = std::make_unique<CircularProvider>();

	entityFilters.push_back(std::make_unique<CollisionFilter>());
}

///////////////////////////////////////////////////////////////////////////////
// BuffMouseClickSkill
///////////////////////////////////////////////////////////////////////////////

BuffMouseClickSkill::BuffMouseClickSkill(SkillParams params, StatModifier statModifier)
	: BuffSkill(params, statModifier)
{
	entityProvider = std::make_unique<MouseClickProvider>();

	entityFilters.push_back(std::make_unique<CollisionFilter>());
}




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// CUSTOM PROJECTILE SKILLS
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// BoneThrowSkill
///////////////////////////////////////////////////////////////////////////////

BoneThrowSkill::BoneThrowSkill(SkillParams params)
	: ProjectileSkill(params)
{}

void BoneThrowSkill::process()
{
	LaunchBoneEvent launchBoneEvent;
	launchBoneEvent.instigator = params.instigator;
	launchBoneEvent.targetPosition = params.targetPosition;
	launchBoneEvent.damage = params.damage;
	launchBoneEvent.collisionMask = params.collidesWith;
	EventSystem<LaunchBoneEvent>::instance().sendEvent(launchBoneEvent);
}
