#pragma once
#include "game/common.hpp"
#include "entities/tiny_ecs.hpp"

struct HPBar
{
	static ECS::Entity createHPBar(vec2 position);
};