#pragma once
#include "game/common.hpp"
#include "entities/tiny_ecs.hpp"

struct CheeseBlob
{
	static ECS::Entity createCheeseBlob(vec2 position);
};
