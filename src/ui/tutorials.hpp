#pragma once
#include "ui_components.hpp"
#include "button.hpp"
#include "effects.hpp"
#include "ui_entities.hpp"

#include "game/common.hpp"
#include "game/event_system.hpp"
#include "game/events.hpp"
#include "game/turn_system.hpp"
#include "entities/tiny_ecs.hpp"
#include "animation/animation_components.hpp"

#include <functional>

class TutorialSystem
{
public:
	TutorialSystem();
	~TutorialSystem();

	static void cleanTutorial();

private:
	EventListenerInfo tutorialStartListener;
	void onTutorialStart(const StartTutorialEvent& event);

	EventListenerInfo tutorialAdvanceListener;
	void onTutorialAdvance(const AdvanceTutorialEvent& event);

	EventListenerInfo tutorialEndListener;
	void onTutorialEnd(const EndTutorialEvent& event);
};

struct TutorialText
{
	static ECS::Entity createTutorialText(vec2 position, int tutorialStage = 0);
};
