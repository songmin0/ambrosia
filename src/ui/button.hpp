#pragma once
#include "game/common.hpp"
#include "entities/tiny_ecs.hpp"
#include "skills/skill_component.hpp"

enum class ButtonShape { CIRCLE, RECTANGLE };

struct Button
{
	static ECS::Entity createButton(ButtonShape shape, vec2 position, const std::string& texture, void(*callback)());
	static ECS::Entity createPlayerButton(PlayerType player, vec2 position, void(*callback)());
};

struct SkillButton
{
	static ECS::Entity createSkillButton(vec2 position, PlayerType player, SkillType skillType, const std::string& texture, void(*callback)());
	static ECS::Entity createMoveButton(vec2 position, const std::string& texture, void(*callback)());
};

struct ToolTip
{
	static ECS::Entity createToolTip(PlayerType player, SkillType skillType, vec2 position = vec2(0.f));
	static ECS::Entity createMoveToolTip(vec2 position = vec2(0.f));
};