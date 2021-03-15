#include "skill.hpp"

#include <game/event_system.hpp>
#include <game/events.hpp>

///////////////////////////////////////////////////////////////////////////////
// Skill
///////////////////////////////////////////////////////////////////////////////

Skill::Skill(SkillParams params)
	: params(params)
{}

void Skill::performSkill(vec2 target)
{
	// Update the internal params
	params.targetPosition = target;

	performSkillInternal();
}

///////////////////////////////////////////////////////////////////////////////
// AreaOfEffectSkill
///////////////////////////////////////////////////////////////////////////////

AreaOfEffectSkill::AreaOfEffectSkill(SkillParams params)
	: Skill(params)
{}

void AreaOfEffectSkill::performSkillInternal()
{
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

ProjectileSkill::ProjectileSkill(SkillParams params, ProjectileType projectileType)
	: Skill(params)
	, projectileType(projectileType)
{}

void ProjectileSkill::performSkillInternal()
{
	LaunchEvent launchEvent;
	launchEvent.projectileType = projectileType;
	launchEvent.instigator = params.instigator;
	launchEvent.targetPosition = params.targetPosition;
	launchEvent.damage = params.damage;
	launchEvent.collisionMask = params.collidesWith;
	launchEvent.callback = [=]() {

		// When the projectile reaches the end of its trajectory, notify the TurnSystem
		// that the skill is finished
		FinishedSkillEvent event;
		event.entity = params.instigator;
		EventSystem<FinishedSkillEvent>::instance().sendEvent(event);
	};
	EventSystem<LaunchEvent>::instance().sendEvent(launchEvent);
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
