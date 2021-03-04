#pragma once
#include "tiny_ecs.hpp"
#include "game/common.hpp"

struct Raoul
{
	static ECS::Entity createRaoul(json initialStats, float colourShift = 0);
	static ECS::Entity Raoul::createRaoul(vec2 position, float colourShift);
};
