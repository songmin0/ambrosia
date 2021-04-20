#include "swarm_behaviour.hpp"
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

	startNextRoundListener = EventSystem<StartNextRoundEvent>::instance().registerListener(
			std::bind(&StatsSystem::onStartNextRoundEvent, this, std::placeholders::_1));

	finishedSkillListener = EventSystem<FinishedSkillEvent>::instance().registerListener(
			std::bind(&StatsSystem::onFinishedSkillEvent, this, std::placeholders::_1));

	prepForNextMapListener = EventSystem<PrepForNextMapEvent>::instance().registerListener(
			std::bind(&StatsSystem::onPrepForNextMapEvent, this, std::placeholders::_1));
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

	if (startNextRoundListener.isValid())
	{
		EventSystem<StartNextRoundEvent>::instance().unregisterListener(startNextRoundListener);
	}

	if (finishedSkillListener.isValid())
	{
		EventSystem<FinishedSkillEvent>::instance().unregisterListener(finishedSkillListener);
	}

	if (prepForNextMapListener.isValid())
	{
		EventSystem<PrepForNextMapEvent>::instance().unregisterListener(prepForNextMapListener);
	}
}

void StatsSystem::addStatModifier(ECS::Entity entity,
																	StatsComponent& statsComponent,
																	StatModifier statModifier)
{
	// These are the only stats that make sense to buff/debuff at the moment
	assert(statModifier.statType == StatType::STRENGTH ||
				 statModifier.statType == StatType::HP_SHIELD ||
				 statModifier.statType == StatType::STUNNED);

	// Check for cc immunity
	if (statModifier.statType == StatType::STUNNED &&
			entity.has<CCImmunityComponent>())
	{
		return;
	}

	// Remove existing modifier if one exists
	removeStatModifier(entity, statsComponent, statModifier.statType);

	// Start the FX
	FXType fxType = getFXType(statModifier);
	EventSystem<StartFXEvent>::instance().sendEvent({entity, fxType});

	// Add the modifier
	statsComponent.statModifiers[statModifier.statType] = statModifier;
}

void StatsSystem::removeStatModifier(ECS::Entity entity,
																		 StatsComponent& statsComponent,
																		 StatType statType)
{
	// Remove the modifier if it exists
	auto it = statsComponent.statModifiers.find(statType);
	if (it != statsComponent.statModifiers.end())
	{
		// Stop the FX
		FXType fxType = getFXType(it->second);
		EventSystem<StopFXEvent>::instance().sendEvent({entity, fxType});

		// Remove the modifier
		statsComponent.statModifiers.erase(it);
	}
}

void StatsSystem::removeAllStatModifiers(ECS::Entity entity,
																				 StatsComponent& statsComponent)
{
	// Stop the FX for each modifier
	for (auto& statModifier : statsComponent.statModifiers)
	{
		FXType fxType = getFXType(statModifier.second);
		EventSystem<StopFXEvent>::instance().sendEvent({entity, fxType});
	}

	// Remove all modifiers
	statsComponent.statModifiers.clear();
}

FXType StatsSystem::getFXType(StatModifier statModifier)
{
	FXType type = FXType::NONE;

	if (statModifier.statType == StatType::STRENGTH)
	{
		type = statModifier.value >= 0.f ? FXType::BUFFED : FXType::DEBUFFED;
	}
	else if (statModifier.statType == StatType::HP_SHIELD)
	{
		type = FXType::SHIELDED;
	}
	else if (statModifier.statType == StatType::STUNNED)
	{
		type = FXType::STUNNED;
	}

	return type;
}

void StatsSystem::handleHitReaction(ECS::Entity entity)
{
	// Activate target's animation
	if (entity.has<AnimationsComponent>())
	{
		auto& animationsComponent = entity.get<AnimationsComponent>();
		animationsComponent.changeAnimation(AnimationType::HIT);
	}

	// Play sound effect
	SoundEffect soundEffect = entity.has<PlayerComponent>() ? SoundEffect::HIT_PLAYER
																													: SoundEffect::HIT_MOB;

	EventSystem<PlaySoundEffectEvent>::instance().sendEvent({soundEffect});
}

void StatsSystem::handleDeathReaction(ECS::Entity entity, StatsComponent& statsComponent)
{
	entity.emplace<DeathTimer>();
	removeAllStatModifiers(entity, statsComponent);

	// Activate target's animation
	if (entity.has<AnimationsComponent>())
	{
		auto& animationsComponent = entity.get<AnimationsComponent>();
		animationsComponent.changeAnimation(AnimationType::DEFEAT);
	}

	// Play sound effect
	EventSystem<PlaySoundEffectEvent>::instance().sendEvent({SoundEffect::DEFEAT});

	// Handle ambrosia
	int ambrosia = (int) statsComponent.getStatValue(StatType::AMBROSIA);
	if (ambrosia > 0)
	{
		EventSystem<AmbrosiaNumberEvent>::instance().sendEvent({entity, ambrosia});
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

	float damageApplied = 0.f;

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

			// Remove HP shield when it runs out
			if (hpShieldAfter == 0.f)
			{
				removeStatModifier(target, statsComponent, StatType::HP_SHIELD);
			}

			// Update the amount of damage that still needs to be applied
			damageApplied = hpShieldBefore - hpShieldAfter;
			actualDamage -= damageApplied;
		}

		// Subtract remaining damage from HP
		float hpBefore = statsComponent.stats[StatType::HP];
		statsComponent.stats[StatType::HP] = std::max(0.f, hpBefore - actualDamage);
		float hpAfter = statsComponent.stats[StatType::HP];

		damageApplied += hpBefore - hpAfter;

		// Check whether the target has died
		if (statsComponent.getEffectiveHP() <= 0.f)
		{
			handleDeathReaction(target, statsComponent);
		}
		else
		{
			handleHitReaction(target);
		}
	}

	// Send event for displaying damage number
	EventSystem<DamageNumberEvent>::instance().sendEvent(DamageNumberEvent{ target, damageApplied });
}

void StatsSystem::onBuffEvent(const BuffEvent &event)
{
	// A character can only be buffed/debuffed when there's no DeathTimer
	auto entity = event.entity;
	if (!entity.has<DeathTimer>() && entity.has<StatsComponent>())
	{
		auto& statsComponent = entity.get<StatsComponent>();

		addStatModifier(event.entity, statsComponent, event.statModifier);
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

		// Start the FX
		EventSystem<StartFXEvent>::instance().sendEvent({entity, FXType::HEALED});
	}
}

void StatsSystem::onStartNextRoundEvent(const StartNextRoundEvent& event)
{
	for (auto entity : ECS::registry<StatsComponent>.entities)
	{
		auto& statsComponent = entity.get<StatsComponent>();
		std::vector<StatType> toRemove;

		for (auto& statModifier : statsComponent.statModifiers)
		{
			// Special handling for stun and HP shield
			if (statModifier.first == StatType::STUNNED ||
					statModifier.first == StatType::HP_SHIELD)
			{
				// Decrement the number of turns remaining before it gets removed
				statModifier.second.numTurns--;
			}

			// Collect the expired modifiers
			if (statModifier.second.numTurns <= 0)
			{
				toRemove.push_back(statModifier.first);
			}
		}

		// Remove the expired modifiers
		for (auto type : toRemove)
		{
			removeStatModifier(entity, statsComponent, type);
		}
	}
}

void StatsSystem::onFinishedSkillEvent(const FinishedSkillEvent& event)
{
	auto entity = event.entity;
	if (entity.has<StatsComponent>())
	{
		auto& statsComponent = entity.get<StatsComponent>();

		for (auto& statModifier : statsComponent.statModifiers)
		{
			if (statModifier.first == StatType::STUNNED ||
					statModifier.first == StatType::HP_SHIELD)
			{
				continue;
			}

			// Decrement the number of turns remaining in which this buff/debuff will
			// affect the entity's skills. If it expires (reaches zero), it will get
			// removed at the start of the next round (see `onStartNextRoundEvent`).
			statModifier.second.numTurns--;
		}
	}
}

void StatsSystem::onPrepForNextMapEvent(const PrepForNextMapEvent& event)
{
	auto entity = event.entity;
	if (entity.has<StatsComponent>())
	{
		auto& statsComponent = entity.get<StatsComponent>();

		// Get rid of existing buffs/debuffs
		removeAllStatModifiers(entity, statsComponent);

		// Restore HP
		statsComponent.stats[StatType::HP] = statsComponent.stats[StatType::MAX_HP];
	}
}