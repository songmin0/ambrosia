#pragma once
#include "tiny_ecs.hpp"
#include "game/common.hpp"

struct Raoul
{
	static ECS::Entity createRaoul(vec2 pos, float colourShift = 0);
};
