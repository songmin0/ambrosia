#include "stats_system.hpp"

StatsSystem::StatsSystem()
{
	hitEventListener = EventSystem<HitEvent>::instance().registerListener(
			std::bind(&StatsSystem::onHitEvent, this, std::placeholders::_1));

	buffEventListener = EventSystem<BuffEvent>::instance().registerListener(
			std::bind(&StatsSystem::onBuffEvent, this, std::placeholders::_1));
}

StatsSystem::~StatsSystem()
{
	if (hitEventListener.isValid())
	{
		EventSystem<HitEvent>::instance().unregisterListener(hitEventListener);
	}

	if (buffEventListener.isValid())
	{
		EventSystem<BuffEvent>::instance().unregisterListener(buffEventListener);
	}
}

void StatsSystem::step(float elapsed_ms)
{
	const float elapsed_s = elapsed_ms / 1000.f;

	// Remove expired stat modifiers
	for (auto& statsComponent : ECS::registry<StatsComponent>.components)
	{
		std::vector<StatType> toRemove;

		// Decrement the StatModifier timers. Add the finished ones to the toRemove list
		for (auto& statModifier : statsComponent.statModifiers)
		{
			statModifier.second.timer -= elapsed_s;

			if (statModifier.second.timer <= 0.f)
			{
				toRemove.push_back(statModifier.first);
			}
		}

		// Remove the expired modifiers
		for (auto type : toRemove)
		{
			statsComponent.statModifiers.erase(type);
		}
	}
}

void StatsSystem::onHitEvent(const HitEvent &event)
{
	auto instigator = event.instigator;
	auto target = event.target;

	float actualDamage = event.damage;

	// Calculate the actual damage based on the instigator's strength
	if (instigator.has<StatsComponent>())
	{
		auto& statsComponent = instigator.get<StatsComponent>();
		float instigatorStrength = statsComponent.getStatValue(StatType::STRENGTH);

		actualDamage *= instigatorStrength;
	}

	// Apply damage to the target
	if (target.has<StatsComponent>())
	{
		auto& statsComponent = target.get<StatsComponent>();

		statsComponent.stats[StatType::HP] -= actualDamage;

		AnimationType animationToPerform = AnimationType::HIT;
		SoundEffect soundEffect = target.has<PlayerComponent>() ? SoundEffect::HIT_PLAYER
																														: SoundEffect::HIT_MOB;

		// Check whether the target has died
		if (statsComponent.stats[StatType::HP] <= 0.f)
		{
			animationToPerform = AnimationType::DEFEAT;

			if (!ECS::registry<DeathTimer>.has(target))
			{
				ECS::registry<DeathTimer>.emplace(target);
				soundEffect = SoundEffect::DEFEAT;
			}
		}

		// Activate target's animation
		if (target.has<AnimationsComponent>())
		{
			auto& animationsComponent = target.get<AnimationsComponent>();
			animationsComponent.changeAnimation(animationToPerform);
		}

		EventSystem<PlaySoundEffectEvent>::instance().sendEvent({soundEffect});
	}


}

void StatsSystem::onBuffEvent(const BuffEvent &event)
{
	auto entity = event.entity;

	if (entity.has<StatsComponent>())
	{
		auto& statsComponent = entity.get<StatsComponent>();

		// Apply the given stat modifier
		statsComponent.statModifiers[event.statModifier.statType] = event.statModifier;
	}
}
