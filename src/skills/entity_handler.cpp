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

		if (fxType == FXType::STUNNED && target.has<TurnSystem::TurnComponent>() && !target.has<CCImmunityComponent>())
		{
			// For now, all stuns are 1 turn and do not stack
			target.get<TurnSystem::TurnComponent>().stunDuration = 1;
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
	statModifier.timer = timer;

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
	statModifier.timer = timer;

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
