#pragma once
#include "tiny_ecs.hpp"
#include "game/common.hpp"

struct Raoul
{
	// Creates all the associated render resources and default transform
	static ECS::Entity createRaoul(vec2 pos, PlayerType player, float colourShift = 0);
};
