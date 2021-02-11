#include "UISystem.hpp"
#include "EventSystem.hpp"

UISystem::UISystem()
{
	mouseClickListener = EventSystem<MouseClickEvent>::Instance().RegisterListener(
		std::bind(&UISystem::OnMouseClick, this, std::placeholders::_1));
}

UISystem::~UISystem()
{
	if (mouseClickListener.IsValid())
	{
		EventSystem<MouseClickEvent>::Instance().UnregisterListener(mouseClickListener);
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
void UISystem::HandleClick(ECS::Entity entity, const MouseClickEvent& event)
{
	auto& clickable = entity.get<ClickableComponent>();
	if (IsClicked(clickable, event.mousePos)) {
		clickable.callback();
	}
}

void UISystem::OnMouseClick(const MouseClickEvent& event)
{
	for (auto entity : ECS::registry<Button>.entities) {
		if (entity.has<ClickableCircleComponent>()) {
			HandleClick<ClickableCircleComponent>(entity, event);
		}
		else if (entity.has<ClickableRectangleComponent>()) {
			HandleClick<ClickableRectangleComponent>(entity, event);
		}
	}
}