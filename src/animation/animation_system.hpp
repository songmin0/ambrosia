#pragma once
#include "game/common.hpp"
#include "rendering/render_components.hpp"
#include "rendering/render.hpp"
#include "game/event_system.hpp"
#include "game/events.hpp"

#include <vector>
#include <unordered_map> 

class AnimationSystem
{
public:
	AnimationSystem();
	~AnimationSystem();

	void step();
	void checkAnimation(ECS::Entity& entity);
	void updateOrientation(Motion& motion, const vec2 direction);

private:
	void onPerformSkillEvent(const PerformActiveSkillEvent& event);
	void onPrepForNextMapEvent(const PrepForNextMapEvent& event);

	EventListenerInfo performSkillListener;
	EventListenerInfo prepForNextMapListener;
};
