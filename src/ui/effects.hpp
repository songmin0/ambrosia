#pragma once
#include "game/common.hpp"
#include "entities/tiny_ecs.hpp"
#include "ui/ui_components.hpp"

struct MouseClickFX
{
	static ECS::Entity createMouseClickFX();
};

struct ActiveSkillFX
{
	static ECS::Entity createActiveSkillFX();
};