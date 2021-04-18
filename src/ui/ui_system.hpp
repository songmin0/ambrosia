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
	static void createCentralMessage(const std::string& message, float durationMS = 3000.f);

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

	EventListenerInfo ambrosiaNumberListener;
	void onAmbrosiaNumberEvent(const AmbrosiaNumberEvent& event);

	EventListenerInfo healEventListener;
	void onHealEvent(const HealEvent& event);

	EventListenerInfo buffEventListener;
	void onBuffEvent(const BuffEvent& event);

	void createDamageNumber(ECS::Entity entity, float value, vec3 color);
	void createAmbrosiaNumber(ECS::Entity entity, int value);
	void launchAmbrosiaProjectile(ECS::Entity entity, int value);
	void playMouseClickFX(vec2 position);
	void updatePlayerSkillButton(ECS::Entity& entity);
	void activateSkillButton(const SkillType& skillType);
	void enableMoveButton(bool doEnable = true);
	void enableSkillButtons(bool doEnable = true);

	void clearToolTips();
	ECS::Entity& getToolTip(SkillType skill);
	void renderToolTipNumbers(const SkillType& skillType);

	const std::vector < std::vector<float> > playerBaseSkillValues{
		{ 30.f, 40.f, 15.f }, // Raoul - S1 dmg, S2 buff%, S3 dmg
		{ 20.f, 20.f, 8.f }, // Taji - S1 dmg, S2 dmg, S3 dmg + heal
		{ 25.f, 50.f, 25.f }, // Ember - S1 dmg, S2 dmg, S3 dmg
		{ 30.f, 20.f, 40.f,  15.f, 30.f, 1.f }, // Chia - S1 heal, S1 dmg, S2 debuff%, S2 dmg, S3 shield, S3 turn duration
	};

	const float NUMSCALE = 0.45f;
};
