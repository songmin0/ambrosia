#pragma once

#include "skill.hpp"

#include "game/event_system.hpp"
#include "game/events.hpp"
#include "game/common.hpp"

class SkillSystem
{
public:
	SkillSystem();
	~SkillSystem();

	void step(float elapsed_ms);

private:
	struct QueuedSkill
	{
		std::shared_ptr<Skill> skill;
		vec2 target;
		float delay;
	};

	void onSetActiveSkillEvent(const SetActiveSkillEvent& event);
	void onPerformSkillEvent(const PerformActiveSkillEvent& event);
	void onPlayerChangeEvent(const PlayerChangeEvent& event);

	std::vector<QueuedSkill> queuedSkills;

	EventListenerInfo setActiveSkillListener;
	EventListenerInfo performSkillListener;
	EventListenerInfo playerChangeListener;
};