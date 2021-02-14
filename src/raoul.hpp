#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"

struct Raoul
{
	// Creates all the associated render resources and default transform
	static ECS::Entity createRaoul(vec2 pos, float colourShift = 0);
};
