#include "entity_handler.hpp"

#include "game/event_system.hpp"
#include "game/events.hpp"
#include "game/turn_system.hpp"
#include "entities/enemies.hpp"

void EntityHandler::process(ECS::Entity instigator, ECS::Entity target)
{
	for (auto fxType : fxTypes)
	{
		if (fxType != FXType::NONE)
		{
			EventSystem<StartFXEvent>::instance().sendEvent({target, fxType});
		}
	}

	processInternal(instigator, target);
}

void DamageHandler::processInternal(ECS::Entity instigator, ECS::Entity target)
{
	HitEvent hitEvent;
	hitEvent.instigator = instigator;
	hitEvent.target = target;
	hitEvent.damage = damage;
	EventSystem<HitEvent>::instance().sendEvent(hitEvent);
}

void HealHandler::processInternal(ECS::Entity instigator, ECS::Entity target)
{
	HealEvent healEvent;
	healEvent.entity = target;
	healEvent.amount = healAmount;
	EventSystem<HealEvent>::instance().sendEvent(healEvent);
}

void BuffHandler::processInternal(ECS::Entity instigator, ECS::Entity target)
{
	StatModifier statModifier;
	statModifier.statType = statType;
	statModifier.value = value;
	statModifier.numTurns = numTurns;

	if (instigator.id == target.id && statType != StatType::HP_SHIELD)
	{
		// Have to sidestep some behaviour here...
		// When an entity buffs/debuffs themselves, the FinishedSkillEvent is going
		// to cause the stat modifier's `numTurns` to decrement by 1 as soon as this
		// skill finishes. To account for that, we have to increment by 1 here.
		statModifier.numTurns++;
	}

	BuffEvent buffEvent;
	buffEvent.entity = target;
	buffEvent.statModifier = statModifier;
	EventSystem<BuffEvent>::instance().sendEvent(buffEvent);
}

void HealAndDamageHandler::processInternal(ECS::Entity instigator, ECS::Entity target)
{
	assert(target.has<Motion>());
	auto& targetMotion = target.get<Motion>();

	if (targetMotion.colliderType == typeToHeal)
	{
		HealEvent healEvent;
		healEvent.entity = target;
		healEvent.amount = healAmount;
		EventSystem<HealEvent>::instance().sendEvent(healEvent);
	}
	else if (targetMotion.colliderType == typeToDamage)
	{
		HitEvent hitEvent;
		hitEvent.instigator = instigator;
		hitEvent.target = target;
		hitEvent.damage = damage;
		EventSystem<HitEvent>::instance().sendEvent(hitEvent);
	}
}

void DebuffAndDamageHandler::processInternal(ECS::Entity instigator, ECS::Entity target)
{
	// DEBUFF
	StatModifier statModifier;
	statModifier.statType = statType;
	statModifier.value = value;
	statModifier.numTurns = numTurns;

	BuffEvent buffEvent;
	buffEvent.entity = target;
	buffEvent.statModifier = statModifier;
	EventSystem<BuffEvent>::instance().sendEvent(buffEvent);

	// DAMAGE
	HitEvent hitEvent;
	hitEvent.instigator = instigator;
	hitEvent.target = target;
	hitEvent.damage = damage;
	EventSystem<HitEvent>::instance().sendEvent(hitEvent);
}

void KnockbackHandler::processInternal(ECS::Entity instigator, ECS::Entity target)
{
	assert(instigator.has<Motion>());
	assert(target.has<Motion>());
	auto& instigatorMotion = instigator.get<Motion>();
	auto& targetMotion = target.get<Motion>();

	// The attack happens from the center of the instigator
	vec2 instigatorCenter = instigatorMotion.position;
	instigatorCenter.y -= abs(instigatorMotion.boundingBox.y / 2.f);

	// The attack is applied to the center of the target
	vec2 targetCenter = targetMotion.position;
	targetCenter.y -= abs(targetMotion.boundingBox.y / 2.f);

	// Calculate target vector, direction, and distance
	vec2 instigatorToTarget = targetCenter - instigatorCenter;
	vec2 directionToTarget = normalize(instigatorToTarget);
	float distanceToTarget = length(instigatorToTarget);

	// Skip target if too far away
	if (distanceToTarget > radius)
	{
		return;
	}

	// The force and damage applied to the target attenuate with distance
	float proportionOfMaxDistance = distanceToTarget / radius;
	float force = mix(maxForce, 0.f, proportionOfMaxDistance);
	float damage = mix(maxDamage, 0.f, proportionOfMaxDistance);

	// Scale the force based on the instigator's strength (the StatsSystem will
	// handle the damage scaling)
	assert(instigator.has<StatsComponent>());
	float instigatorStrength = instigator.get<StatsComponent>().getStatValue(StatType::STRENGTH);
	float actualForce = force * instigatorStrength;

	// Calculate the impulse (magnitude + direction)
	vec2 impulse = directionToTarget * actualForce;

	// Apply impulse to the target
	ImpulseEvent impulseEvent;
	impulseEvent.entity = target;
	impulseEvent.impulse = impulse;
	EventSystem<ImpulseEvent>::instance().sendEvent(impulseEvent);

	// Damage the target
	HitEvent hitEvent;
	hitEvent.instigator = instigator;
	hitEvent.target = target;
	hitEvent.damage = damage;
	EventSystem<HitEvent>::instance().sendEvent(hitEvent);
}
