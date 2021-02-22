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
	void onHitEvent(const HitEvent& event);
	void onBuffEvent(const BuffEvent& event);

	EventListenerInfo hitEventListener;
	EventListenerInfo buffEventListener;
};
