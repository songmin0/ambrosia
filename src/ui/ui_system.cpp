#include "ui_system.hpp"
#include "game/camera.hpp"
#include <game/game_state_system.hpp>
#include <iostream>

UISystem::UISystem()
{
	mouseClickListener = EventSystem<RawMouseClickEvent>::instance().registerListener(
		std::bind(&UISystem::onMouseClick, this, std::placeholders::_1));

	playerChangeListener = EventSystem<PlayerChangeEvent>::instance().registerListener(
		std::bind(&UISystem::onPlayerChange, this, std::placeholders::_1));

	mouseHoverListener = EventSystem<RawMouseHoverEvent>::instance().registerListener(
		std::bind(&UISystem::onMouseHover, this, std::placeholders::_1));

	activateSkillListener = EventSystem<SetActiveSkillEvent>::instance().registerListener(
		std::bind(&UISystem::onSkillActivate, this, std::placeholders::_1));

	finishedSkillListener = EventSystem<FinishedSkillEvent>::instance().registerListener(
		std::bind(&UISystem::onSkillFinished, this, std::placeholders::_1));

	finishedMovementListener = EventSystem<FinishedMovementEvent>::instance().registerListener(
		std::bind(&UISystem::onMoveFinished, this, std::placeholders::_1));
}

UISystem::~UISystem()
{
	if (mouseClickListener.isValid()) {
		EventSystem<RawMouseClickEvent>::instance().unregisterListener(mouseClickListener);
	}

	if (playerChangeListener.isValid()) {
		EventSystem<PlayerChangeEvent>::instance().unregisterListener(playerChangeListener);
	}

	if (mouseHoverListener.isValid()) {
		EventSystem<RawMouseHoverEvent>::instance().unregisterListener(mouseHoverListener);
	}

	if (activateSkillListener.isValid()) {
		EventSystem<SetActiveSkillEvent>::instance().unregisterListener(activateSkillListener);
	}

	if (finishedSkillListener.isValid()) {
		EventSystem<FinishedSkillEvent>::instance().unregisterListener(finishedSkillListener);
	}

	if (finishedMovementListener.isValid()) {
		EventSystem<FinishedMovementEvent>::instance().unregisterListener(finishedMovementListener);
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

	// Filters mouse clicks through ClickFilters first - only clicks within a filter can continue
	if (!ECS::registry<ClickFilter>.entities.empty())
	{
		bool didPassFilter = false;
		for (auto entity : ECS::registry<ClickFilter>.entities)
		{
			// filters can "absorb" clicks, in which case they are handled by the filter callback and not passed further
			// create a copy in case handling the click removes the filter
			bool doesAbsorbClick = entity.get<ClickFilter>().doAbsorbClick;
			if (handleClick<ClickableRectangleComponent>(entity, event) && !doesAbsorbClick)
			{
				didPassFilter = true;
			}
		}

		// Only proceed to handle mouse click if it passes a filter
		if (!didPassFilter)
		{
			return;
		}
	}

	// When in help overlay, only listen for clicks on the help button
	if (!ECS::registry<HelpOverlay>.entities.empty())
	{
		for (auto entity : ECS::registry<HelpButton>.entities)
		{
			handleClick<ClickableRectangleComponent>(entity, event);
		}
		return;
	}

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

	// Sends a MouseClickEvent to event system if no buttons are clicked, takes into account the camera position
	assert(!ECS::registry<CameraComponent>.entities.empty());
	auto camera = ECS::registry<CameraComponent>.entities[0];
	auto& cameraPos = camera.get<CameraComponent>().position;
	EventSystem<MouseClickEvent>::instance().sendEvent(MouseClickEvent{ event.mousePos + cameraPos });
}

void UISystem::enableMoveButton(bool doEnable)
{
	auto& moveButtonState = ECS::registry<MoveButtonComponent>.entities.front().get<ButtonStateComponent>();
	moveButtonState.isDisabled = !doEnable;
	moveButtonState.isActive = doEnable;
}

void UISystem::enableSkillButtons(bool doEnable)
{
	for (auto& entity : ECS::registry<SkillButton>.entities)
	{
		assert(entity.has<ButtonStateComponent>() && entity.has<SkillInfoComponent>());
		auto& buttonState = entity.get<ButtonStateComponent>();
		if (entity.get<SkillInfoComponent>().skillType != SkillType::MOVE)
		{
			buttonState.isActive = doEnable;
			buttonState.isDisabled = !doEnable;
		}
	}
}

void UISystem::updatePlayerSkillButton(ECS::Entity& entity)
{
	if (entity.has<PlayerComponent>() && entity.has< TurnSystem::TurnComponent>())
	{
		const auto& player = entity.get<PlayerComponent>().player;
		auto& turnComponent = entity.get<TurnSystem::TurnComponent>();

		// If the player is dead or they completed their turn, don't update
		if (entity.has<DeathTimer>() || TurnSystem::hasCompletedTurn(turnComponent))
		{
			return;
		}

		// update the button textures to that player
		for (auto& buttonInfo : ECS::registry<SkillInfoComponent>.components)
		{
			buttonInfo.player = player;
		}

		// sync buttons to current action
		if (turnComponent.canStartMoving()) // for now, this means they can't use skills yet
		{
			enableMoveButton(true);
			enableSkillButtons(false);
			activateSkillButton(SkillType::MOVE);
		}
		else if (turnComponent.canStartSkill())
		{
			enableMoveButton(false);
			enableSkillButtons(true);
			const auto& skillType = entity.get<SkillComponent>().getActiveSkillType();
			activateSkillButton(skillType);
		}
	}
	else
	{
		enableMoveButton(false);
		enableSkillButtons(false);
	}
}

void UISystem::onMoveFinished(const FinishedMovementEvent& event)
{
	auto entity = event.entity;
	if (entity.has<SkillComponent>() && entity.has<PlayerComponent>())
	{
		enableMoveButton(false);
		enableSkillButtons(true);
		const auto& skillType = entity.get<SkillComponent>().getActiveSkillType();
		activateSkillButton(skillType);
	}
}

// Handles highlighting player buttons on player change and swapping skill UI
void UISystem::onPlayerChange(const PlayerChangeEvent& event)
{
	// update skill UI
	auto newPlayer = event.newActiveEntity;
	updatePlayerSkillButton(newPlayer);

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

	// Create active arrow when a player is active
	if (newPlayer.has<PlayerComponent>() && ECS::registry<ActiveArrow>.entities.empty()) {
		ActiveArrow::createActiveArrow();
	}
	// Remove active arrow when a mob is active
	else if (!newPlayer.has<PlayerComponent>() && !ECS::registry<ActiveArrow>.entities.empty()) {
		ECS::ContainerInterface::removeAllComponentsOf(ECS::registry<ActiveArrow>.entities.back());
	}

	int tutorialIndex = GameStateSystem::instance().currentTutorialIndex;
	if (GameStateSystem::instance().isInTutorial 
		&& (tutorialIndex == 6 || tutorialIndex == 7 || tutorialIndex == 9))
	{
		EventSystem<AdvanceTutorialEvent>::instance().sendEvent(AdvanceTutorialEvent{});
	}
}

ECS::Entity& UISystem::getToolTip(SkillType skill)
{
	if (skill == SkillType::MOVE)
	{
		assert(ECS::registry<MoveToolTipComponent>.size() > 0);
		return ECS::registry<MoveToolTipComponent>.entities.front();
	}

	for (auto& entity : ECS::registry<ToolTip>.entities)
	{
		auto skillInfo = entity.get<SkillInfoComponent>();
		if (skillInfo.skillType == skill)
		{
			return entity;
		}
	}

	// temporary default tooltip
	assert(ECS::registry<MoveToolTipComponent>.size() > 0);
	return ECS::registry<MoveToolTipComponent>.entities.front();
}

void UISystem::clearToolTips()
{
	for (auto entity : ECS::registry<ToolTip>.entities)
	{
		if (entity.has<VisibilityComponent>())
		{
			entity.get<VisibilityComponent>().isVisible = false;
		}
	}
}

void printSkillButtonHoverDebug(PlayerType player, SkillType skillType)
{
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

	switch (skillType) {
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
	std::cout << "hovering over: " << playerName << "'s " << skillName << " button" << std::endl;
}

void UISystem::onMouseHover(const RawMouseHoverEvent& event)
{
	bool didTriggerTooltip = false;
	for (auto entity : ECS::registry<SkillButton>.entities) {
		assert(entity.has<ClickableCircleComponent>());

		auto& clickableArea = entity.get<ClickableCircleComponent>();
		if (isClicked(clickableArea, event.mousePos))
		{
			didTriggerTooltip = true;
			auto& skillInfo = entity.get<SkillInfoComponent>();
			SkillType skillType = skillInfo.skillType;
			PlayerType player = skillInfo.player;

			ECS::Entity& toolTip = getToolTip(skillType);
			auto& vis = toolTip.get<VisibilityComponent>();
			vis.isVisible = true;

			//printSkillButtonHoverDebug(player, skillType);
		}
	}

	if (!didTriggerTooltip)
	{
		clearToolTips();
	}
	// Sends a MouseHoverEvent to event system that takes into account the camera position
	assert(!ECS::registry<CameraComponent>.entities.empty());
	auto camera = ECS::registry<CameraComponent>.entities[0];
	auto& cameraPos = camera.get<CameraComponent>().position;
	EventSystem<MouseHoverEvent>::instance().sendEvent(MouseHoverEvent{ event.mousePos + cameraPos });
}

void UISystem::activateSkillButton(const SkillType& skillType)
{
	for (auto& entity : ECS::registry<SkillButton>.entities)
	{
		assert(entity.has<SkillInfoComponent>() && entity.has<ButtonStateComponent>());
		auto skillInfo = entity.get<SkillInfoComponent>();
		auto& buttonState = entity.get<ButtonStateComponent>();
		if (skillInfo.skillType == skillType && !buttonState.isDisabled)
		{
			buttonState.isActive = true;

			assert(ECS::registry<ActiveSkillFX>.entities.size() > 0);
			auto& activeFX = ECS::registry<ActiveSkillFX>.entities.front();
			activeFX.get<Motion>().position = entity.get<Motion>().position;
			activeFX.get<VisibilityComponent>().isVisible = true;
		}
		else
		{
			buttonState.isActive = false;
		}
	}
}

void UISystem::onSkillActivate(const SetActiveSkillEvent& event)
{
	auto player = event.entity;
	if (!player.has<PlayerComponent>())
	{
		return;
	}

	auto& turnComponent = player.get<TurnSystem::TurnComponent>();
	if (turnComponent.canStartSkill())
	{
		activateSkillButton(event.type);
	}
}

void UISystem::onSkillFinished(const FinishedSkillEvent& event)
{
	auto player = event.entity;
	if (!player.has<PlayerComponent>())
	{
		return;
	}

	for (auto& entity : ECS::registry<SkillButton>.entities)
	{
		assert(entity.has<ButtonStateComponent>());
		auto& buttonState = entity.get<ButtonStateComponent>();
		if (!buttonState.isDisabled)
		{
			buttonState.isActive = false;

			assert(ECS::registry<ActiveSkillFX>.entities.size() > 0);
			auto& activeFX = ECS::registry<ActiveSkillFX>.entities.front();
			activeFX.get<VisibilityComponent>().isVisible = false;
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
			entity.get<AnimationsComponent>().currAnimData->currFrame = 0;
			break;
		}
	}
}