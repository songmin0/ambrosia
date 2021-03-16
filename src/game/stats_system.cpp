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

				// If the target's HP is buffed or debuffed, remove the stat modifier
				// now so that we don't end up with a situation where the stat modifier
				// gets removed in StatsSystem::step while the DeathTimer is counting
				// down, causing the HP to change. It would be weird to see the HP bar
				// change after death.
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
	assert(event.statModifier.statType != StatType::INVALID);

	auto entity = event.entity;

	// A character can only be buffed/debuffed when there's no DeathTimer
	if (!entity.has<DeathTimer>() && entity.has<StatsComponent>())
	{
		auto& statsComponent = entity.get<StatsComponent>();

		// When applying an HP modifier, make sure the HP stays between 1.f and MAXHP
		StatModifier statModifier = event.statModifier;
		if (statModifier.statType == StatType::HP)
		{
			float currentHP = statsComponent.stats[StatType::HP];
			float modifiedHP = currentHP + statModifier.value;

			// Prevent debuffs from killing the character. Please feel free to remove
			// this if we decide that debuffs should be able to kill.
			if (modifiedHP < 1.f)
			{
				statModifier.value = -currentHP + 1.f;
			}
			// Prevent buffing the character's HP beyond the maximum
			else
			{
				float maxHP = statsComponent.stats[StatType::MAXHP];
				if (modifiedHP > maxHP)
				{
					statModifier.value = maxHP - currentHP;
				}
			}
		}

		// Apply the given stat modifier
		statsComponent.statModifiers[event.statModifier.statType] = statModifier;
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
		float maxHP = statsComponent.getStatValue(StatType::MAXHP);

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
