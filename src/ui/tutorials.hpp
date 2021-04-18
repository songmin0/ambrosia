#pragma once
#include "ui_entities.hpp"
#include "game/common.hpp"
#include "game/event_system.hpp"
#include "game/events.hpp"
#include "entities/tiny_ecs.hpp"

#include <functional>

class TutorialSystem
{
public:
	TutorialSystem();
	~TutorialSystem();

	static void cleanTutorial();
	static void toggleInspectMode();

private:
	EventListenerInfo tutorialStartListener;
	void onTutorialStart(const StartTutorialEvent& event);

	EventListenerInfo tutorialAdvanceListener;
	void onTutorialAdvance(const AdvanceTutorialEvent& event);

	EventListenerInfo tutorialEndListener;
	void onTutorialEnd(const EndTutorialEvent& event);

	EventListenerInfo showHelpListener;
	void onShowHelp(const ShowHelpEvent& event);

	EventListenerInfo hideHelpListener;
	void onHideHelp(const HideHelpEvent& event);

	EventListenerInfo advanceStoryListener;
	void onAdvanceStory(const AdvanceStoryEvent& event);

	EventListenerInfo onMouseHoverListener;
	void onMouseHover(const MouseHoverEvent& event);

	void showInspectToolTip(ECS::Entity & entity);
};

struct TutorialText
{
	static ECS::Entity createTutorialText(vec2 position, int tutorialStage = 0);
};

struct StoryScene
{
	static ECS::Entity createStoryScene(vec2 screenSize, int storyStage, bool hasAmbrosia = false);
};
