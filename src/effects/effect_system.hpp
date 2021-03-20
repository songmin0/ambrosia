#pragma once
#include "game/common.hpp"
#include "game/event_system.hpp"
#include "game/events.hpp"

class EffectSystem
{
public:
	EffectSystem();
	~EffectSystem();

	void step();

private:
	EventListenerInfo startFXListener;
	void onStartFX(const StartFXEvent& event);

	EventListenerInfo stopFXListener;
	void onStopFX(const StopFXEvent& event);

	void setBuffedOffsetAndScale(ECS::Entity entity, ECS::Entity fxEntity);
	void setStunnedOffsetAndScale(ECS::Entity entity, ECS::Entity fxEntity);
	void setBlueberriedOffset(ECS::Entity entity, ECS::Entity fxEntity);
	void setCandyOffset(ECS::Entity entity, ECS::Entity fxEntity);

	// Used to assign an order to skillFX so render order does not change if an entity has multple FXs
	static unsigned int nextOrderId() {
		static unsigned int order = 1;
		return order++;
	}
};