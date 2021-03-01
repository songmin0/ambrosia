#pragma once
#include "tiny_ecs.hpp"
#include "game/common.hpp"

struct Taji
{
	static ECS::Entity createTaji(vec2 pos, float colourShift = 0);
};
