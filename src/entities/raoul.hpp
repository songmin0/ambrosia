#pragma once
#include "tiny_ecs.hpp"
#include "game/common.hpp"

struct Raoul
{
	static ECS::Entity createRaoul(json initialStats, float colourShift = 0);
};
