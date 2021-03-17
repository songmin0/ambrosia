#pragma once
#include "game/common.hpp"
#include "entities/tiny_ecs.hpp"
#include "skills/skill_component.hpp"
#include "ui_components.hpp"

struct HPBar
{
	static ECS::Entity createHPBar(vec2 position, vec2 scale = vec2(0.5f, 0.4f));
	vec2 offset = vec2(0.f);
	ECS::Entity statsCompEntity;
	bool isMob = false;
};

struct ToolTip
{
	static ECS::Entity createToolTip(PlayerType player, SkillType skillType, vec2 position = vec2(0.f));
	static ECS::Entity createMoveToolTip(vec2 position = vec2(0.f));
};

struct TajiHelper { static ECS::Entity createTajiHelper(vec2 position); };

struct ClickFilter { static ECS::Entity createClickFilter(vec2 position, bool isLarge = false, vec2 scale = vec2(1.f)); };