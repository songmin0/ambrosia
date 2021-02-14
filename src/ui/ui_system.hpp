#pragma once
#include "ui_components.hpp"
#include "button.hpp"

#include "game/common.hpp"
#include "game/event_system.hpp"
#include "game/events.hpp"
#include "entities/tiny_ecs.hpp"

#include <functional>

class UISystem
{
public:
	UISystem();
	~UISystem();

private:
	EventListenerInfo mouseClickListener;

	bool isClicked(ClickableCircleComponent clickable, vec2 position);
	bool isClicked(ClickableRectangleComponent clickable, vec2 position);

	template <typename ClickableComponent>
	bool handleClick(ECS::Entity entity, const RawMouseClickEvent& event);

	void onMouseClick(const RawMouseClickEvent& event);
};
