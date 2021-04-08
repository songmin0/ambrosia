#pragma once
#include "ui_components.hpp"
#include "button.hpp"
#include "effects.hpp"
#include "ui_entities.hpp"

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

	void step(float elapsed_ms);

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

	EventListenerInfo activateSkillListener;
	void onSkillActivate(const SetActiveSkillEvent& event);

	EventListenerInfo finishedSkillListener;
	void onSkillFinished(const FinishedSkillEvent& event);

	EventListenerInfo finishedMovementListener;
	void onMoveFinished(const FinishedMovementEvent& event);

	EventListenerInfo damageNumberEventListener;
	void onDamageNumberEvent(const DamageNumberEvent& event);

	EventListenerInfo healEventListener;
	void onHealEvent(const HealEvent& event);

	EventListenerInfo buffEventListener;
	void onBuffEvent(const BuffEvent& event);

	void createDamageNumber(ECS::Entity entity, float value, vec3 color);
	void playMouseClickFX(vec2 position);
	void updatePlayerSkillButton(ECS::Entity& entity);
	void activateSkillButton(const SkillType& skillType);
	void enableMoveButton(bool doEnable = true);
	void enableSkillButtons(bool doEnable = true);
	void enableActiveSkillFX(bool doEnable = true);

	void clearToolTips();
	ECS::Entity& getToolTip(SkillType skill);
};
