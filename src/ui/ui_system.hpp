#pragma once
#include "ui_components.hpp"
#include "button.hpp"
#include "effects.hpp"

#include "game/common.hpp"
#include "game/event_system.hpp"
#include "game/events.hpp"
#include "game/turn_system.hpp"
#include "entities/tiny_ecs.hpp"
#include "animation/animation_components.hpp"

#include <functional>

class UISystem
{
public:
	UISystem();
	~UISystem();

private:
	bool isClicked(ClickableCircleComponent clickable, vec2 position);
	bool isClicked(ClickableRectangleComponent clickable, vec2 position);

	template <typename ClickableComponent>
	bool handleClick(ECS::Entity entity, const RawMouseClickEvent& event);

	EventListenerInfo mouseClickListener;
	void onMouseClick(const RawMouseClickEvent& event);

	EventListenerInfo playerChangeListener;
	void onPlayerChange(const PlayerChangeEvent& event);

	EventListenerInfo mouseHoverListener;
	void onMouseHover(const RawMouseHoverEvent& event);

	EventListenerInfo activateSkillListener;
	void onSkillActivate(const SetActiveSkillEvent& event);
	ECS::Entity& getSkillButton(PlayerType player, SkillType skill);

	EventListenerInfo finishedSkillListener;
	void onSkillFinished(const FinishedSkillEvent& event);

	void playMouseClickFX(vec2 position);
	void updatePlayerSkillButtons(const PlayerType& player);

	void clearToolTips();
	ECS::Entity& getToolTip(SkillType skill);
};
