#include "ui_system.hpp"
#include <iostream>

UISystem::UISystem()
{
	mouseClickListener = EventSystem<RawMouseClickEvent>::instance().registerListener(
		std::bind(&UISystem::onMouseClick, this, std::placeholders::_1));

	playerChangeListener = EventSystem<PlayerChangeEvent>::instance().registerListener(
		std::bind(&UISystem::onPlayerChange, this, std::placeholders::_1));

	mouseHoverListener = EventSystem<RawMouseHoverEvent>::instance().registerListener(
		std::bind(&UISystem::onMouseHover, this, std::placeholders::_1));
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
	// Plays a mouse click fx
	playMouseClickFX(event.mousePos);

	// Handles if any button entities are clicked
	for (auto entity : ECS::registry<Button>.entities) {
		if (handleClick<ClickableCircleComponent>(entity, event)) {
			return;
		}
		else if (handleClick<ClickableRectangleComponent>(entity, event)) {
			return;
		}
	}

	for (auto entity : ECS::registry<SkillButton>.entities) {
		if (handleClick<ClickableCircleComponent>(entity, event)) {
			return;
		}
	}

	// Sends a MouseClickEvent to event system if no buttons are clicked
	EventSystem<MouseClickEvent>::instance().sendEvent(MouseClickEvent{ event.mousePos });
}

// Handles highlighting player buttons on player change
void UISystem::onPlayerChange(const PlayerChangeEvent& event)
{
	// Go through all the buttons and update their animations
	for (auto playerButton : ECS::registry<PlayerButtonComponent>.entities)
	{
		auto& animComponent = playerButton.get<AnimationsComponent>();
		auto& playerButtonComponent = playerButton.get<PlayerButtonComponent>();

		// Find the player associated with this button
		bool playerFound = false;
		for (auto player : ECS::registry<PlayerComponent>.entities)
		{
			auto& playerComponent = player.get<PlayerComponent>();

			// Check whether this is the player associated with the button
			if (playerComponent.player == playerButtonComponent.player)
			{
				auto& turnComponent = player.get<TurnSystem::TurnComponent>();

				// If the player is dead or they completed their turn, show disabled button
				if (player.has<DeathTimer>() || TurnSystem::hasCompletedTurn(turnComponent))
				{
					animComponent.changeAnimation(AnimationType::DISABLED);
				}
				// If it's the player's turn, show active button
				else if (player.id == event.newActiveEntity.id)
				{
					animComponent.changeAnimation(AnimationType::ACTIVE);
				}
				// Otherwise show inactive button
				else
				{
					animComponent.changeAnimation(AnimationType::INACTIVE);
				}
				playerFound = true;
				break;
			}
		}

		// If the player wasn't found, then the player is probably dead and has been removed from the registry
		if (!playerFound)
		{
			animComponent.changeAnimation(AnimationType::DISABLED);
		}
	}
}

void UISystem::onMouseHover(const RawMouseHoverEvent& event)
{
	for (auto entity : ECS::registry<SkillButton>.entities) {
		assert(entity.has<ClickableCircleComponent>());

		auto& clickableArea = entity.get<ClickableCircleComponent>();
		if (isClicked(clickableArea, event.mousePos))
		{
			auto& skillInfo = entity.get<SkillButtonComponent>();
			SkillType skillType = skillInfo.skillType;
			PlayerType player = skillInfo.player;
			std::string playerName = "???";
			std::string skillName = "???";

			switch (player) {
			case PlayerType::RAOUL:
				playerName = "raoul";
				break;
			case PlayerType::TAJI:
				playerName = "taji";
				break;
			case PlayerType::CHIA:
				playerName = "chia";
				break;
			case PlayerType::EMBER:
				playerName = "ember";
				break;
			default:
				break;
			}

			switch (skillType){
			case SkillType::MOVE:
				skillName = "move";
				break;
			case SkillType::SKILL1:
				skillName = "skill 1";
				break;
			case SkillType::SKILL2:
				skillName = "skill 2";
				break;
			case SkillType::SKILL3:
				skillName = "skill 3";
				break;
			case SkillType::SKILL4:
				skillName = "skill 4";
				break;
			default:
				break;
			}
			
			std::cout << "hovering over: " << playerName << "'s " << skillName << " button /n";
		}
	}
}


void UISystem::playMouseClickFX(vec2 position)
{
	for (auto& entity : ECS::registry<MouseClickFX>.entities)
	{
		if (entity.has<AnimationsComponent>() && entity.has<Motion>())
		{
			entity.get<Motion>().position = position;
			entity.get<AnimationsComponent>().currAnimData.currFrame = 0;
		}
	}
}