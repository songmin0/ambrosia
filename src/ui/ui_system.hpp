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

	void playMouseClickFX(vec2 position);

	void clearToolTips();
	ECS::Entity& getToolTip(PlayerType player, SkillType skill);
};
