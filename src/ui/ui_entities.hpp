#pragma once
#include "game/common.hpp"
#include "entities/tiny_ecs.hpp"
#include "skills/skill_component.hpp"
#include "ui_components.hpp"

struct HPBar
{
	static ECS::Entity createHPBar(vec2 position);
};

struct ToolTip
{
	static ECS::Entity createToolTip(PlayerType player, SkillType skillType, vec2 position = vec2(0.f));
	static ECS::Entity createMoveToolTip(vec2 position = vec2(0.f));
};