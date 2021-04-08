#pragma once

#include "common.hpp"
#include "event_system.hpp"
#include "events.hpp"

class StatsSystem
{
public:
	StatsSystem();
	~StatsSystem();

private:
	void addStatModifier(ECS::Entity entity,
											 StatsComponent& statsComponent,
											 StatModifier statModifier);
	void removeStatModifier(ECS::Entity entity,
													StatsComponent& statsComponent,
													StatType statType);
	void removeAllStatModifiers(ECS::Entity entity,
															StatsComponent& statsComponent);
	FXType getFXType(StatModifier statModifier);
	void handleHitReaction(ECS::Entity entity);
	void handleDeathReaction(ECS::Entity entity, StatsComponent& statsComponent);

	void onHitEvent(const HitEvent& event);
	void onBuffEvent(const BuffEvent& event);
	void onHealEvent(const HealEvent& event);
	void onStartNextRoundEvent(const StartNextRoundEvent& event);
	void onFinishedSkillEvent(const FinishedSkillEvent& event);
	void onPrepForNextMapEvent(const PrepForNextMapEvent& event);

	EventListenerInfo hitEventListener;
	EventListenerInfo buffEventListener;
	EventListenerInfo healEventListener;
	EventListenerInfo startNextRoundListener;
	EventListenerInfo finishedSkillListener;
	EventListenerInfo prepForNextMapListener;
};
