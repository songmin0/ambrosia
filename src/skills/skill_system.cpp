#include "skill_system.hpp"
#include "skill_component.hpp"

SkillSystem::SkillSystem()
{
	setActiveSkillListener = EventSystem<SetActiveSkillEvent>::instance().registerListener(
			std::bind(&SkillSystem::onSetActiveSkillEvent, this, std::placeholders::_1));

	performSkillListener = EventSystem<PerformActiveSkillEvent>::instance().registerListener(
			std::bind(&SkillSystem::onPerformSkillEvent, this, std::placeholders::_1));

	playerChangeListener = EventSystem<PlayerChangeEvent>::instance().registerListener(
			std::bind(&SkillSystem::onPlayerChangeEvent, this, std::placeholders::_1));
}

SkillSystem::~SkillSystem()
{
	if (setActiveSkillListener.isValid())
	{
		EventSystem<SetActiveSkillEvent>::instance().unregisterListener(setActiveSkillListener);
	}

	if (performSkillListener.isValid())
	{
		EventSystem<PerformActiveSkillEvent>::instance().unregisterListener(performSkillListener);
	}

	if (playerChangeListener.isValid())
	{
		EventSystem<PlayerChangeEvent>::instance().unregisterListener(playerChangeListener);
	}
}

void SkillSystem::step(float elapsed_ms)
{
	const float elapsed_s = elapsed_ms / 1000.f;
	std::vector<int> toRemove;

	// Go through the queued skills and execute the ones whose timer have reached zero. The purpose of the timer is to
	// allow us to sync up the animation with the execution of the skill
	for (int i = 0; i < queuedSkills.size(); i++)
	{
		auto& queuedSkill = queuedSkills[i];

		queuedSkill.delay -= elapsed_s;
		if (queuedSkill.delay <= 0.f)
		{
			// If the timer has reached zero, perform the skill and get ready to remove it from the queuedSkills list
			queuedSkill.skill->performSkill(queuedSkill.target);
			toRemove.push_back(i);

			// Play sound effect
			EventSystem<PlaySoundEffectEvent>::instance().sendEvent({queuedSkill.skill->getSoundEffect()});

			// AreaOfEffect skills happen immediately, so we can notify the TurnSystem that the skill is done right away.
			// Projectile skills are not done until the projectile reaches the end of its trajectory, so we don't send a
			// FinishedSkillEvent for those ones here. The ProjectileSystem handles that.
			if (std::dynamic_pointer_cast<AreaOfEffectSkill>(queuedSkill.skill))
			{
				// Notify the TurnSystem that this entity has performed a skill
				FinishedSkillEvent event;
				event.entity = queuedSkill.skill->getInstigator();
				EventSystem<FinishedSkillEvent>::instance().sendEvent(event);
			}
		}
	}

	// Remove the skills that were performed
	for (int i = toRemove.size() - 1; i >= 0; i--)
	{
		queuedSkills.erase(queuedSkills.begin() + toRemove[i]);
	}
}

// Every time a skill button is pressed, we change the active skill
void SkillSystem::onSetActiveSkillEvent(const SetActiveSkillEvent &event)
{
	auto entity = event.entity;

	if (entity.has<SkillComponent>())
	{
		entity.get<SkillComponent>().setActiveSkill(event.type);
	}
}

void SkillSystem::onPerformSkillEvent(const PerformActiveSkillEvent &event)
{
	auto entity = event.entity;

	if (entity.has<SkillComponent>())
	{
		std::shared_ptr<Skill> activeSkill = entity.get<SkillComponent>().getActiveSkill();

		if (activeSkill)
		{
			// Start the animation immediately
			activeSkill->performAnimation();

			// Queue the skill so that it gets executed once the timer reaches zero (to allow for syncing up the animation
			// with the execution of the skill)
			QueuedSkill queuedSkill;
			queuedSkill.skill = activeSkill;
			queuedSkill.target = event.target;
			queuedSkill.delay = activeSkill->getDelay();

			queuedSkills.push_back(queuedSkill);
		}
	}
}

// When a turn starts, set that entity's selected skill back to SKILL1
void SkillSystem::onPlayerChangeEvent(const PlayerChangeEvent &event)
{
	auto entity = event.newActiveEntity;

	if (entity.has<SkillComponent>())
	{
		entity.get<SkillComponent>().setActiveSkill(SkillType::SKILL1);
	}
}
