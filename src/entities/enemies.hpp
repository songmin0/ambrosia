#pragma once
#include "tiny_ecs.hpp"
#include "game/common.hpp"

struct Pepper
{
	static ECS::Entity createPepper(vec2 pos);
};

struct Milk
{
	static ECS::Entity createMilk(vec2 pos, float orientation = 1.f);
};
