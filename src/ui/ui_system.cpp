#include "ui_system.hpp"

UISystem::UISystem()
{
	mouseClickListener = EventSystem<RawMouseClickEvent>::instance().registerListener(
		std::bind(&UISystem::onMouseClick, this, std::placeholders::_1));

	playerChangeListener = EventSystem<PlayerChangeEvent>::instance().registerListener(
		std::bind(&UISystem::onPlayerChange, this, std::placeholders::_1));
}

UISystem::~UISystem()
{
	if (mouseClickListener.isValid()) {
		EventSystem<RawMouseClickEvent>::instance().unregisterListener(mouseClickListener);
	}

	if (playerChangeListener.isValid()) {
		EventSystem<PlayerChangeEvent>::instance().unregisterListener(playerChangeListener);
	}
}

bool UISystem::isClicked(ClickableCircleComponent clickable, vec2 position)
{
	// Checks if given position is within the circle radius
	float distance = sqrt(pow((clickable.position.x - position.x), 2) + pow((clickable.position.y - position.y), 2));
	return distance <= clickable.radius;
}

bool UISystem::isClicked(ClickableRectangleComponent clickable, vec2 position)
{
	// Check if given position is within the rectangle area
	return abs(clickable.position.x - position.x) <= clickable.width / 2
		&& abs(clickable.position.y - position.y) <= clickable.height / 2;
}

template <typename ClickableComponent>
// Handles calling callback function when a button is clicked
// returns true if button is clicked, false otherwise
bool UISystem::handleClick(ECS::Entity entity, const RawMouseClickEvent& event)
{
	if (!entity.has<ClickableComponent>()) {
		return false;
	}

	auto& clickable = entity.get<ClickableComponent>();
	if (isClicked(clickable, event.mousePos)) {
		clickable.callback();
		return true;
	}

	return false;
}

void UISystem::onMouseClick(const RawMouseClickEvent& event)
{
	// Handles if any button entities are clicked
	for (auto entity : ECS::registry<Button>.entities) {
		if (handleClick<ClickableCircleComponent>(entity, event)) {
			return;
		}
		else if (handleClick<ClickableRectangleComponent>(entity, event)) {
			return;
		}
	}

	// Sends a MouseClickEvent to event system if no buttons are clicked
	EventSystem<MouseClickEvent>::instance().sendEvent(MouseClickEvent{ event.mousePos });
}

// Handles highlighting player buttons on player change
void UISystem::onPlayerChange(const PlayerChangeEvent& event)
{
	for (auto entity : ECS::registry<PlayerComponent>.entities) {
		PlayerType player = entity.get<PlayerComponent>().player;

		// find the player's corresponding player button
		for (auto playerButton : ECS::registry<PlayerButtonComponent>.entities) {
			auto& playerButtonComponent = playerButton.get<PlayerButtonComponent>();
			if (player != playerButtonComponent.player) {
				continue;
			}

			auto& anim = playerButton.get<AnimationsComponent>();
			// show active button if player is active
			if (entity.has<TurnSystem::TurnComponentIsActive>()) {
				anim.changeAnimation(AnimationType::ACTIVE);
				break;
			}

			auto& turnComponent = entity.get<TurnSystem::TurnComponent>();
			// show disable button if player has completed their turn
			if (TurnSystem::hasCompletedTurn(turnComponent)) {
				anim.changeAnimation(AnimationType::DISABLED);
			}
			else { // show inactive button
				anim.changeAnimation(AnimationType::INACTIVE);
			}
			break;
		}
	}
}