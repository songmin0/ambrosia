#pragma once
#include "tiny_ecs.hpp"
#include "game/common.hpp"

struct Chia
{
	static ECS::Entity createChia(json initialStats);
	static ECS::Entity createChia(vec2 position);
};
