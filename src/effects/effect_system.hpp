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

	void setBuffedOffsetAndScale(ECS::Entity fxEntity);
	void setStunnedOffsetAndScale(ECS::Entity fxEntity);
	void setBlueberriedOffset(ECS::Entity fxEntity);
	void setCandyOffset(ECS::Entity fxEntity);
};