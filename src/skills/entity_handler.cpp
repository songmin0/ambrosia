#include "entity_handler.hpp"

#include "game/event_system.hpp"
#include "game/events.hpp"

void DamageHandler::process(ECS::Entity instigator, ECS::Entity target)
{
	HitEvent hitEvent;
	hitEvent.instigator = instigator;
	hitEvent.target = target;
	hitEvent.damage = damage;
	EventSystem<HitEvent>::instance().sendEvent(hitEvent);
}

void HealHandler::process(ECS::Entity instigator, ECS::Entity target)
{
	HealEvent healEvent;
	healEvent.entity = target;
	healEvent.amount = healAmount;
	EventSystem<HealEvent>::instance().sendEvent(healEvent);
}

void BuffHandler::process(ECS::Entity instigator, ECS::Entity target)
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
