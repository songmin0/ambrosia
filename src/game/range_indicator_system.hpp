#include "game/event_system.hpp"
#include "game/events.hpp"
#include "game/common.hpp"

class RangeIndicatorSystem {
public:
	RangeIndicatorSystem();
	~RangeIndicatorSystem();
private:
	void onRangeIndicatorChange(const ChangeRangeIndicatorEvent& event);
	void performActiveSkillEvent(const PerformActiveSkillEvent& event);
	void mouseHoverEvent(const MouseHoverEvent event);
	void onPlayerChange(const PlayerChangeEvent event);

	EventListenerInfo skillChangeListener;
	EventListenerInfo performActiveSkillListener;
	EventListenerInfo playerChangeListener;

	bool activeSkillUsesMouseLoc = false;
	vec2 curMousePos;

	ECS::Entity rangeIndicator;
};