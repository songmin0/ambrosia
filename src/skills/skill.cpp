#include "skill.hpp"

#include "game/event_system.hpp"
#include "game/events.hpp"
#include <utility>

///////////////////////////////////////////////////////////////////////////////
// Skill
///////////////////////////////////////////////////////////////////////////////

Skill::Skill(std::shared_ptr<SkillParams> params)
	: params(std::move(params))
{}

void Skill::performSkill(vec2 target)
{
	assert(params);

	// Update the internal params
	params->targetPosition = target;

	// Play sound effect
	EventSystem<PlaySoundEffectEvent>::instance().sendEvent({params->soundEffect});

	// Reset filters
	for (auto& filter : params->entityFilters)
	{
		assert(filter);
		filter->reset();
	}

	// Perform the skill
	performSkillInternal();
}

float Skill::getDelay() const
{
	assert(params);
	return params->delay;
}

AnimationType Skill::getAnimationType() const
{
	assert(params);
	return params->animationType;
}

std::shared_ptr<SkillParams> Skill::getParams()
{
	return params;
}


///////////////////////////////////////////////////////////////////////////////
// AreaOfEffectSkill
///////////////////////////////////////////////////////////////////////////////

AreaOfEffectSkill::AreaOfEffectSkill(std::shared_ptr<AoESkillParams> params)
	: Skill(std::move(params))
{}

float AreaOfEffectSkill::getRange()
{
	if (auto aoeParams = std::dynamic_pointer_cast<AoESkillParams>(params))
	{
		assert(aoeParams->entityProvider);
		assert(aoeParams->entityHandler);
		//TODO handle different providers
		if (auto circularProvider = std::dynamic_pointer_cast<CircularProvider>(aoeParams->entityProvider)) {
			return circularProvider->getRadius();
		}
		else if (auto mouseClickProvider = std::dynamic_pointer_cast<MouseClickProvider>(aoeParams->entityProvider)) {
			return mouseClickProvider->getRadius();
		}
	}
	return 0.0f;
}

void AreaOfEffectSkill::performSkillInternal()
{
	assert(params);

	if (auto aoeParams = std::dynamic_pointer_cast<AoESkillParams>(params))
	{
		assert(aoeParams->entityProvider);
		assert(aoeParams->entityHandler);

		// Collect the entities
		std::vector<ECS::Entity> entities
				= aoeParams->entityProvider->getEntities(aoeParams->instigator,
																								 aoeParams->targetPosition);

		for (auto entity : entities)
		{
			if (entity.has<DeathTimer>())
			{
				continue;
			}

			// Run the entity through the filters. A filter returns false if the
			// entity should get filtered out
			bool shouldKeepEntity = true;
			for (auto& filter : aoeParams->entityFilters)
			{
				assert(filter);
				if (!filter->process(entity))
				{
					shouldKeepEntity = false;
					break;
				}
			}

			// If the entity made it through the filters successfully, apply the
			// entity handler
			if (shouldKeepEntity)
			{
				aoeParams->entityHandler->process(aoeParams->instigator, entity);
			}
		}
	}

	// Notify the TurnSystem that the skill is finished
	EventSystem<FinishedSkillEvent>::instance().sendEvent({params->instigator});
}

///////////////////////////////////////////////////////////////////////////////
// ProjectileSkill
///////////////////////////////////////////////////////////////////////////////

ProjectileSkill::ProjectileSkill(std::shared_ptr<ProjectileSkillParams> params)
	: Skill(std::move(params))
{}

float ProjectileSkill::getRange()
{
	return 0.0f;
}

void ProjectileSkill::performSkillInternal()
{
	assert(params);

	if (auto projParams = std::dynamic_pointer_cast<ProjectileSkillParams>(params))
	{
		LaunchEvent launchEvent;
		launchEvent.skillParams = *projParams;

		launchEvent.callback = [=]() {
			// When the projectile reaches the end of its trajectory, notify the TurnSystem
			// that the skill is finished
			EventSystem<FinishedSkillEvent>::instance().sendEvent({params->instigator});
		};

		EventSystem<LaunchEvent>::instance().sendEvent(launchEvent);
	}
}
