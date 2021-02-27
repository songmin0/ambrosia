#pragma once
#include "game/common.hpp"
#include "entities/tiny_ecs.hpp"

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

struct SkillButtonComponent
{
	SkillButtonComponent() : isActive{ false }, isDisabled{ false }, isEnabled{ true } {}
	SkillButtonComponent(bool active, bool disabled, bool enabled) : isActive{ active }, isDisabled{ disabled }, isEnabled{ enabled } {}
	bool isActive;
	bool isDisabled;
	bool isEnabled;
};