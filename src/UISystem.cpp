#include "UISystem.hpp"
#include "EventSystem.hpp"

UISystem::UISystem()
{
	mouseClickListener = EventSystem<RawMouseClickEvent>::Instance().RegisterListener(
		std::bind(&UISystem::OnMouseClick, this, std::placeholders::_1));
}

UISystem::~UISystem()
{
	if (mouseClickListener.IsValid())
	{
		EventSystem<RawMouseClickEvent>::Instance().UnregisterListener(mouseClickListener);
	}
}

bool UISystem::IsClicked(ClickableCircleComponent clickable, vec2 position)
{
	// Checks if given position is within the circle radius
	float distance = sqrt(pow((clickable.position.x - position.x), 2) + pow((clickable.position.y - position.y), 2));
	return distance <= clickable.radius;
}

bool UISystem::IsClicked(ClickableRectangleComponent clickable, vec2 position)
{
	// Check if given position is within the rectangle area
	return abs(clickable.position.x - position.x) <= clickable.width / 2
		&& abs(clickable.position.y - position.y) <= clickable.height / 2;
}

template <typename ClickableComponent>
// Handles calling callback function when a button is clicked
// returns true if button is clicked, false otherwise
bool UISystem::HandleClick(ECS::Entity entity, const RawMouseClickEvent& event)
{
	if (!entity.has<ClickableComponent>()) {
		return false;
	}

	auto& clickable = entity.get<ClickableComponent>();
	if (IsClicked(clickable, event.mousePos)) {
		clickable.callback();
		return true;
	}

	return false;
}

void UISystem::OnMouseClick(const RawMouseClickEvent& event)
{
	// Handles if any button entities are clicked
	for (auto entity : ECS::registry<Button>.entities) {
		if (HandleClick<ClickableCircleComponent>(entity, event)) {
			return;
		}
		else if (HandleClick<ClickableRectangleComponent>(entity, event)) {
			return;
		}
	}

	// Sends a MouseClickEvent to event system if no buttons are clicked
	EventSystem<MouseClickEvent>::Instance().SendEvent(MouseClickEvent{ event.mousePos });
}