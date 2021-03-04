#pragma once
#include "tiny_ecs.hpp"
#include "game/common.hpp"

struct Taji
{
	static ECS::Entity createTaji(json initialStats, float colourShift = 0);
	static ECS::Entity Taji::createTaji(vec2 position, float colourShift);
};
