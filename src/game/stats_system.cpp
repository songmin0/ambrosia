#include "stats_system.hpp"
#include "game/game_state_system.hpp"

StatsSystem::StatsSystem()
{
	hitEventListener = EventSystem<HitEvent>::instance().registerListener(
			std::bind(&StatsSystem::onHitEvent, this, std::placeholders::_1));

	buffEventListener = EventSystem<BuffEvent>::instance().registerListener(
			std::bind(&StatsSystem::onBuffEvent, this, std::placeholders::_1));

	healEventListener = EventSystem<HealEvent>::instance().registerListener(
			std::bind(&StatsSystem::onHealEvent, this, std::placeholders::_1));
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

	if (healEventListener.isValid())
	{
		EventSystem<HealEvent>::instance().unregisterListener(healEventListener);
	}
}

void StatsSystem::step(float elapsed_ms)
{
		if (GameStateSystem::instance().inGameState()) {
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
}

void StatsSystem::onHitEvent(const HitEvent &event)
{
	auto instigator = event.instigator;
	auto target = event.target;

	// Don't apply damage if target is already dead
	if (target.has<DeathTimer>())
	{
		return;
	}

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

		// Apply damage to HP shield first. The HP_SHIELD stat itself should always
		// be zero. We only need to check the stat modifier.
		auto it = statsComponent.statModifiers.find(StatType::HP_SHIELD);
		if (it != statsComponent.statModifiers.end())
		{
			// Apply damage to HP shield
			float hpShieldBefore = it->second.value;
			it->second.value = std::max(0.f, hpShieldBefore - actualDamage);
			float hpShieldAfter = it->second.value;

			// Update the amount of damage that still needs to be applied
			actualDamage -= hpShieldBefore - hpShieldAfter;
		}

		// Subtract remaining damage from HP
		float currentHP = statsComponent.stats[StatType::HP];
		statsComponent.stats[StatType::HP] = std::max(0.f, currentHP - actualDamage);

		AnimationType animationToPerform = AnimationType::HIT;
		SoundEffect soundEffect = target.has<PlayerComponent>() ? SoundEffect::HIT_PLAYER
																														: SoundEffect::HIT_MOB;

		// Check whether the target has died
		if (statsComponent.getEffectiveHP() <= 0.f)
		{
			animationToPerform = AnimationType::DEFEAT;

			if (!ECS::registry<DeathTimer>.has(target))
			{
				ECS::registry<DeathTimer>.emplace(target);
				soundEffect = SoundEffect::DEFEAT;
				statsComponent.statModifiers.erase(StatType::HP);
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
	// These are the only stats that make sense to buff/debuff at the moment
	assert(event.statModifier.statType == StatType::STRENGTH ||
				 event.statModifier.statType == StatType::MAX_HP ||
				 event.statModifier.statType == StatType::HP_SHIELD);

	auto entity = event.entity;

	// A character can only be buffed/debuffed when there's no DeathTimer
	if (!entity.has<DeathTimer>() && entity.has<StatsComponent>())
	{
		auto& statsComponent = entity.get<StatsComponent>();

		// Apply the given stat modifier
		statsComponent.statModifiers[event.statModifier.statType] = event.statModifier;
	}
}

// A HealEvent increases the character's HP. If the character's HP is currently
// buffed, and the buff will now cause the character to have more than the
// maximum HP, the buff gets adjusted so that it only buffs up to the maximum.
void StatsSystem::onHealEvent(const HealEvent &event)
{
	// A HealEvent should never deal damage
	assert(event.amount >= 0.f);

	auto entity = event.entity;

	// Healing can only be applied when there's no DeathTimer
	if (!entity.has<DeathTimer>() && entity.has<StatsComponent>())
	{
		auto& statsComponent = entity.get<StatsComponent>();

		// Get the current HP
		float currentHP = statsComponent.stats[StatType::HP];

		// Get the maximum possible HP
		float maxHP = statsComponent.getStatValue(StatType::MAX_HP);

		// Apply the healing, keeping the HP within the maximum
		float newHP = std::min(currentHP + event.amount, maxHP);
		statsComponent.stats[StatType::HP] = newHP;

		// If a stat modifier is currently applied, update its value if it causes
		// the HP to go above the maximum
		auto it = statsComponent.statModifiers.find(StatType::HP);
		if (it != statsComponent.statModifiers.end() && (newHP + it->second.value > maxHP))
		{
			it->second.value = maxHP - newHP;
		}
	}
}
