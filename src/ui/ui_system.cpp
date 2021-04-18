#include "ui_system.hpp"
#include "game/camera.hpp"
#include "game/game_state_system.hpp"
#include "rendering/text.hpp"
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

	damageNumberEventListener = EventSystem<DamageNumberEvent>::instance().registerListener(
		std::bind(&UISystem::onDamageNumberEvent, this, std::placeholders::_1));

	healEventListener = EventSystem<HealEvent>::instance().registerListener(
		std::bind(&UISystem::onHealEvent, this, std::placeholders::_1));

	buffEventListener = EventSystem<BuffEvent>::instance().registerListener(
		std::bind(&UISystem::onBuffEvent, this, std::placeholders::_1));

	ambrosiaNumberListener = EventSystem<AmbrosiaNumberEvent>::instance().registerListener(
			std::bind(&UISystem::onAmbrosiaNumberEvent, this, std::placeholders::_1));
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

	if (damageNumberEventListener.isValid()) {
		EventSystem<DamageNumberEvent>::instance().unregisterListener(damageNumberEventListener);
	}

	if (healEventListener.isValid()) {
		EventSystem<HealEvent>::instance().unregisterListener(healEventListener);
	}

	if (buffEventListener.isValid()) {
		EventSystem<BuffEvent>::instance().unregisterListener(buffEventListener);
	}

	if (ambrosiaNumberListener.isValid()) {
		EventSystem<AmbrosiaNumberEvent>::instance().unregisterListener(ambrosiaNumberListener);
	}
}

void UISystem::step(float elapsed_ms) {
	// Remove damage number once timer expires
	for (auto entity : ECS::registry<TimedUIComponent>.entities) {
		auto& timedUIComponent = entity.get<TimedUIComponent>();
		timedUIComponent.timerMs -= elapsed_ms;

		if (timedUIComponent.timerMs < 0) {
			if (entity.has<AmbrosiaNumberComponent>())
			{
				// Before removing the ambrosia icon, create an ambrosia projectile which
				// travels to the top-left corner of the screen
				int ambrosiaAmount = entity.get<AmbrosiaNumberComponent>().ambrosiaAmount;
				launchAmbrosiaProjectile(entity, ambrosiaAmount);
			}

			ECS::ContainerInterface::removeAllComponentsOf(entity);
		}
	}
}

void UISystem::createCentralMessage(const std::string& text, float durationMS)
{
	// Only show the most current message at any one time
	for (auto& entity : ECS::registry<CentralMessageComponent>.entities)
	{
		ECS::ContainerInterface::removeAllComponentsOf(entity);
	}

	auto str_len = text.length();
	int firstUnprintedChar = 0;
	int yPos = 130;
	int xPos = 320;
	int lineWidth = 45;
	int yPadding = 30;
	float scale = 0.5;

	for (int i = lineWidth; i < str_len; i += lineWidth) {
		while (!isspace(text.at(i))) {
			i--;
		}

		if (isspace(text.at(i))) {
			auto message = createText(text.substr(firstUnprintedChar, i - firstUnprintedChar + 1), { xPos, yPos += yPadding }, scale);
			message.emplace<TimedUIComponent>(durationMS);
			message.emplace<CentralMessageComponent>();
			firstUnprintedChar = i + 1;
		}
	}

	auto message = createText(text.substr(firstUnprintedChar, str_len - firstUnprintedChar + 1), { xPos, yPos += yPadding }, scale);
	message.emplace<TimedUIComponent>(durationMS);
	message.emplace<CentralMessageComponent>();
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

	// When in inspect mode
	if (GameStateSystem::instance().isInspecting)
	{
		for (auto entity : ECS::registry<InspectButton>.entities)
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

	for (auto entity : ECS::registry<UpgradeButton>.entities) {
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
	moveButtonState.isActive = false;
	assert(ECS::registry<ActiveSkillFX>.entities.size() > 0);
	auto& activeFX = ECS::registry<ActiveSkillFX>.entities.front();
	activeFX.get<VisibilityComponent>().isVisible = false;
}

void UISystem::enableSkillButtons(bool doEnable)
{
	for (auto& entity : ECS::registry<SkillButton>.entities)
	{
		assert(entity.has<ButtonStateComponent>() && entity.has<SkillInfoComponent>());
		auto& buttonState = entity.get<ButtonStateComponent>();
		if (entity.get<SkillInfoComponent>().skillType != SkillType::MOVE)
		{
			buttonState.isActive = false;
			buttonState.isDisabled = !doEnable;
		}
	}

	assert(ECS::registry<ActiveSkillFX>.entities.size() > 0);
	auto& activeFX = ECS::registry<ActiveSkillFX>.entities.front();
	activeFX.get<VisibilityComponent>().isVisible = false;
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

		// Null State depending on what is available
		if (turnComponent.canStartMoving())
		{
			enableMoveButton(true);
		}
		else // disable move button
		{
			enableMoveButton(false);
		}
		if (turnComponent.canStartSkill())
		{
			enableSkillButtons(true);
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
		assert(ECS::registry<ActiveSkillFX>.entities.size() > 0);
		auto& activeFX = ECS::registry<ActiveSkillFX>.entities.front();
		activeFX.get<VisibilityComponent>().isVisible = false;
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

	for (auto entity : ECS::registry<ToolTipText>.entities)
	{
		ECS::ContainerInterface::removeAllComponentsOf(entity);
	}
}

void UISystem::onMouseHover(const RawMouseHoverEvent& event)
{
	clearToolTips();
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

			renderToolTipNumbers(skillType);
		}
	}

	// Sends a MouseHoverEvent to event system that takes into account the camera position
	assert(!ECS::registry<CameraComponent>.entities.empty());
	auto camera = ECS::registry<CameraComponent>.entities[0];
	auto& cameraPos = camera.get<CameraComponent>().position;
	EventSystem<MouseHoverEvent>::instance().sendEvent(MouseHoverEvent{ event.mousePos + cameraPos });
}

void UISystem::renderToolTipNumbers(const SkillType& skillType)
{
	ECS::Entity activePlayer = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];

	if (!activePlayer.has<PlayerComponent>() || activePlayer.has<DeathTimer>()
		|| !activePlayer.has<SkillComponent>() || !activePlayer.has<StatsComponent>())
	{
		return;
	}

	if (skillType == SkillType::NONE || skillType == SkillType::MOVE)
	{
		return;
	}

	auto skillLevelMultiplier = activePlayer.get<SkillComponent>().getSkillLevel(skillType) - 1;
	const PlayerType player = activePlayer.get<PlayerComponent>().player;
	auto strengthMultiplier = activePlayer.get<StatsComponent>().getStatValue(StatType::STRENGTH);

	switch (player) {
	case PlayerType::RAOUL:
		if (skillType == SkillType::SKILL1)
		{
			float dmg = playerBaseSkillValues[0][0] + skillLevelMultiplier * 5.f;
			dmg *= strengthMultiplier;
			createText(floatToString(dmg), vec2(297.f, 665.f), NUMSCALE - 0.03, RED).emplace<ToolTipText>();
		}
		else if (skillType == SkillType::SKILL2)
		{
			float buffAmount = playerBaseSkillValues[0][1] + skillLevelMultiplier * 10.f;
			createText(floatToString(buffAmount) + "%", vec2(490.f, 665.f), NUMSCALE - 0.05f, PINK).emplace<ToolTipText>();
		}
		else if (skillType == SkillType::SKILL3)
		{
			float dmg = playerBaseSkillValues[0][2] + skillLevelMultiplier * 5.f;
			dmg *= strengthMultiplier;
			createText(floatToString(dmg), vec2(602.f, 705.f), NUMSCALE - 0.1f, RED).emplace<ToolTipText>();
		}
		break;
	case PlayerType::TAJI:
		if (skillType == SkillType::SKILL1)
		{
			float dmg = playerBaseSkillValues[1][0] + skillLevelMultiplier * 5.f;
			dmg *= strengthMultiplier;
			createText(floatToString(dmg), vec2(313.f, 665.f), NUMSCALE, RED).emplace<ToolTipText>();
		}
		else if (skillType == SkillType::SKILL2)
		{
			float dmg = playerBaseSkillValues[1][1] + skillLevelMultiplier * 5.f;
			dmg *= strengthMultiplier;
			float duration = skillLevelMultiplier == 2 ? 2.f : 1.f; // 2 turns at max level, else 1 turn
			createText(floatToString(dmg), vec2(452.f, 654.f), NUMSCALE - 0.05f, RED).emplace<ToolTipText>();
			createText(floatToString(duration), vec2(438.f, 741.f), NUMSCALE, CYAN).emplace<ToolTipText>();
		}
		else if (skillType == SkillType::SKILL3)
		{
			float dmg = playerBaseSkillValues[1][2] + skillLevelMultiplier * 4.f;
			float heal = dmg; // heals do not scale off strength 
			dmg *= strengthMultiplier;
			createText(floatToString(dmg), vec2(586.f, 663.f), NUMSCALE - 0.1f, RED).emplace<ToolTipText>();
			createText(floatToString(heal), vec2(634.f, 703.f), NUMSCALE - 0.1f, GREEN).emplace<ToolTipText>();
		}
		break;
	case PlayerType::EMBER:
		if (skillType == SkillType::SKILL1)
		{
			float dmg = playerBaseSkillValues[2][0] + skillLevelMultiplier * 5.f;
			dmg *= strengthMultiplier;
			createText(floatToString(dmg), vec2(313.f, 663.f), NUMSCALE, RED).emplace<ToolTipText>();
		}
		else if (skillType == SkillType::SKILL2)
		{
			float dmg = playerBaseSkillValues[2][1] + skillLevelMultiplier * 8.f;
			dmg *= strengthMultiplier;
			createText(floatToString(dmg), vec2(452.f, 663.f), NUMSCALE - 0.05f, RED).emplace<ToolTipText>();
		}
		else if (skillType == SkillType::SKILL3)
		{
			float dmg = playerBaseSkillValues[2][2] + skillLevelMultiplier * 5.f;
			dmg *= strengthMultiplier;
			createText(floatToString(dmg), vec2(600.f, 657.f), NUMSCALE - 0.07f, RED).emplace<ToolTipText>();
		}
		break;
	case PlayerType::CHIA:
		if (skillType == SkillType::SKILL1)
		{
			float dmg = playerBaseSkillValues[3][1] + skillLevelMultiplier * 10.f;
			dmg *= strengthMultiplier;
			float heal = playerBaseSkillValues[3][0] + skillLevelMultiplier * 10.f;
			createText(floatToString(dmg), vec2(345.f, 712.f), NUMSCALE - 0.1f, RED).emplace<ToolTipText>();
			createText(floatToString(heal), vec2(298.f, 675.f), NUMSCALE - 0.1f, GREEN).emplace<ToolTipText>();
		}
		else if (skillType == SkillType::SKILL2)
		{
			float debuffAmount = playerBaseSkillValues[3][2] + skillLevelMultiplier * 10.f;
			float dmg = playerBaseSkillValues[3][3] + skillLevelMultiplier * 5.f;
			dmg *= strengthMultiplier;
			createText(floatToString(dmg), vec2(445.f, 662.f), NUMSCALE - 0.07f, RED).emplace<ToolTipText>();
			createText(floatToString(debuffAmount) + "%", vec2(390.f, 704.f), NUMSCALE - 0.07f, PINK).emplace<ToolTipText>();
		}
		else if (skillType == SkillType::SKILL3)
		{
			float shieldAmount = playerBaseSkillValues[3][4] + skillLevelMultiplier * 10.f;
			float duration = playerBaseSkillValues[3][5] + skillLevelMultiplier;
			createText(floatToString(shieldAmount), vec2(655.f, 695.f), NUMSCALE - 0.06f, CYAN).emplace<ToolTipText>();
			createText(floatToString(duration), vec2(595.f, 716.f), NUMSCALE, CYAN).emplace<ToolTipText>();
		}
		break;
	default:
		break;
	}
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

	activateSkillButton(event.type);
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

void UISystem::onDamageNumberEvent(const DamageNumberEvent& event) {
	createDamageNumber(event.target, event.damage, vec3(0.9f, 0.f, 0.f));
}

void UISystem::onAmbrosiaNumberEvent(const AmbrosiaNumberEvent& event) {
	createAmbrosiaNumber(event.entity, event.amount);
}

void UISystem::onHealEvent(const HealEvent& event) {
	createDamageNumber(event.entity, event.amount, vec3(0.f, 0.9f, 0.f));
}

void UISystem::onBuffEvent(const BuffEvent& event) {
	// Create damage number for hp shield buff
	if (event.statModifier.statType == StatType::HP_SHIELD) {
		createDamageNumber(event.entity, event.statModifier.value, vec3(0.f, 0.9f, 0.9f));
	}
}

void UISystem::createDamageNumber(ECS::Entity entity, float value, vec3 color) {
	auto& motion = entity.get<Motion>();
	std::string number = std::to_string((int)value);
	// Position the damage number using the target's hp bar offset
	auto offset = entity.get<StatsComponent>().healthBar.get<HPBar>().offset;
	offset += vec2(-20, -20); // Hardcoded offset

	auto text = createText(number, motion.position + offset, 1.f, color);
	text.emplace<DamageNumberComponent>(offset);
	text.emplace<TimedUIComponent>(2500.f);
}

void UISystem::createAmbrosiaNumber(ECS::Entity entity, int value)
{
	auto& motion = entity.get<Motion>();
	std::string number = std::to_string(value);

	// Position the ambrosia number using the target's hp bar offset
	auto hpBarOffset = entity.get<StatsComponent>().healthBar.get<HPBar>().offset;

	// Create the text
	static constexpr vec2 TEXT_OFFSET(-20.f, 65.f);
	static constexpr float TEXT_SCALE(0.8f);

	auto textPosition = motion.position + hpBarOffset + TEXT_OFFSET;
	auto text = createText(number, textPosition, TEXT_SCALE, AMBROSIA_COLOUR);
	text.emplace<DamageNumberComponent>(TEXT_OFFSET);
	text.emplace<TimedUIComponent>(2500.f);

	// Create the icon
	static constexpr vec2 ICON_OFFSET(-50.f, 55.f);
	static constexpr vec2 ICON_SCALE(0.7f);

	auto iconPosition = motion.position + hpBarOffset + ICON_OFFSET;
	auto iconEntity = AmbrosiaIcon::createAmbrosiaIcon(iconPosition, ICON_SCALE);
	iconEntity.emplace<RenderableComponent>(RenderLayer::PROJECTILE);
	iconEntity.emplace<AmbrosiaNumberComponent>(value);
	iconEntity.emplace<TimedUIComponent>(1000.f);
}

void UISystem::launchAmbrosiaProjectile(ECS::Entity entity, int value)
{
	if (ECS::registry<AmbrosiaDisplay>.entities.empty())
	{
		return;
	}

	auto depositAmbrosia = [=]()
	{
		EventSystem<DepositAmbrosiaEvent>::instance().sendEvent({value});
	};

	auto getAmbrosiaDisplayPos = []() -> vec2
	{
		if (!ECS::registry<CameraComponent>.entities.empty())
		{
			auto camera = ECS::registry<CameraComponent>.entities.front();
			auto cameraPos = camera.get<CameraComponent>().position;
			return cameraPos + AMBROSIA_DISPLAY_OFFSET;
		}
		return vec2(0.f);
	};

	ProjectileSkillParams skillParams;
	skillParams.projectileType = ProjectileType::AMBROSIA_ICON;
	skillParams.instigator = entity;
	skillParams.targetPosition = getAmbrosiaDisplayPos();

	LaunchEvent launchEvent;
	launchEvent.skillParams = skillParams;
	launchEvent.callback = depositAmbrosia;
	launchEvent.targetPositionProvider = getAmbrosiaDisplayPos;

	EventSystem<LaunchEvent>::instance().sendEvent(launchEvent);
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