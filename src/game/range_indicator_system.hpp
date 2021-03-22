#include "game/event_system.hpp"
#include "game/events.hpp"
#include "game/common.hpp"

class RangeIndicatorSystem {
public:
	RangeIndicatorSystem();
private:
	void skillActiveEvent(const SetActiveSkillEvent& event);
	void performAcvtiveSkillEvent(const PerformActiveSkillEvent& event);
	void mouseHoverEvent(const MouseHoverEvent event);
	

	EventListenerInfo skillAvtiveListener;
	EventListenerInfo performActiveSkillListener;

	bool activeSkillUsesMouseLoc;
	vec2 curMousePos;

	ECS::Entity rangeIndicator;
};