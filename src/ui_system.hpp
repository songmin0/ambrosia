#pragma once
#include "common.hpp"
#include <functional>
#include "tiny_ecs.hpp"
#include "ui_components.hpp"
#include "button.hpp"
#include "events.hpp"
#include "event_system.hpp"

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
