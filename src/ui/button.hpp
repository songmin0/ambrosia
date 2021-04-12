#pragma once
#include "game/common.hpp"
#include "entities/tiny_ecs.hpp"
#include "skills/skill_component.hpp"

enum class ButtonShape { CIRCLE, RECTANGLE };

struct Button
{
	static ECS::Entity createButton(ButtonShape shape, vec2 position, const std::string& texture, void(*callback)());
	static ECS::Entity createPlayerButton(PlayerType player, vec2 position, void(*callback)());
	static ECS::Entity createPlayerUpgradeButton(ButtonShape shape, vec2 position, const std::string& texture, void(*callback)());
};

struct SkillButton
{
	static ECS::Entity createSkillButton(vec2 position, PlayerType player, SkillType skillType, const std::string& texture, void(*callback)());
	static ECS::Entity createMoveButton(vec2 position, const std::string& texture, void(*callback)());
};

struct UpgradeButton
{
	static ECS::Entity createUpgradeButton(vec2 position, PlayerType player, SkillType skillType, const std::string& texture, void(*callback)());
};