#pragma once

#include "common.hpp"
#include "event_system.hpp"
#include "events.hpp"

class StatsSystem
{
public:
	StatsSystem();
	~StatsSystem();

	void step(float elapsed_ms);

private:
	void addStatModifier(ECS::Entity entity,
											 StatsComponent& statsComponent,
											 StatModifier statModifier);

	void removeStatModifier(ECS::Entity entity,
													StatsComponent& statsComponent,
													StatType statType);

	FXType getFXType(StatModifier statModifier);

	void onHitEvent(const HitEvent& event);
	void onBuffEvent(const BuffEvent& event);
	void onHealEvent(const HealEvent& event);

	EventListenerInfo hitEventListener;
	EventListenerInfo buffEventListener;
	EventListenerInfo healEventListener;
};
