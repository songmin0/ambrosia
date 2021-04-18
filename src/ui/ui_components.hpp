#pragma once
#include "game/common.hpp"
#include "entities/tiny_ecs.hpp"
#include "skills/skill_component.hpp"
#include "button.hpp"

struct ClickableCircleComponent
{
	vec2 position = { 0, 0 };
	float radius = 0;
	void (*callback)();
	ClickableCircleComponent(vec2 position, float radius, void (*callback)());
};

struct ClickableRectangleComponent
{
	vec2 position = { 0, 0 };
	float width = 0;
	float height = 0;
	void (*callback)();
	ClickableRectangleComponent(vec2 position, float width, float height, void (*callback)());
};

struct PlayerButtonComponent
{
	PlayerType player;
	PlayerButtonComponent(PlayerType player);
};

struct ButtonStateComponent
{
	ButtonStateComponent() : isActive{ false }, isDisabled{ false } {}
	ButtonStateComponent(bool active, bool disabled) : isActive{ active }, isDisabled{ disabled } {}
	bool isActive;
	bool isDisabled;
};

struct SkillInfoComponent
{
	SkillInfoComponent() : player{ PlayerType::RAOUL }, skillType{ SkillType::SKILL1 } {}
	SkillInfoComponent(PlayerType playerType, SkillType skill) : player{ playerType }, skillType{ skill } {}
	PlayerType player;
	SkillType skillType;
};

struct TimedUIComponent
{
	TimedUIComponent(float timerMs) : timerMs{ timerMs } {}
	float timerMs;
};

struct DamageNumberComponent
{
	DamageNumberComponent(vec2 offset) : offset{ offset } {}
	vec2 offset;
};

struct AmbrosiaNumberComponent
{
	AmbrosiaNumberComponent(int ambrosiaAmount) : ambrosiaAmount{ ambrosiaAmount } {}
	int ambrosiaAmount;
};

// ECS-style Button Labels
struct UIComponent {};
struct MoveToolTipComponent {};
struct MoveButtonComponent {};
struct TutorialComponent {};
struct StoryComponent {};
struct CentralMessageComponent {};
struct ToolTipText {};
