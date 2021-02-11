#pragma once
#include "common.hpp"
#include <functional>
#include "tiny_ecs.hpp"
#include "ui_components.hpp"
#include "Button.hpp"
#include "Events.hpp"
#include "EventSystem.hpp"

class UISystem
{
public:
	UISystem();
	~UISystem();

private:
	EventListenerInfo mouseClickListener;

	bool IsClicked(ClickableCircleComponent clickable, vec2 position);
	bool IsClicked(ClickableRectangleComponent clickable, vec2 position);

	template <typename ClickableComponent>
	void HandleClick(ECS::Entity entity, const MouseClickEvent& event);

	void OnMouseClick(const MouseClickEvent& event);
};
