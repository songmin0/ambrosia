#pragma once
#include "tiny_ecs.hpp"
#include "game/common.hpp"

struct Ember
{
	static ECS::Entity createEmber(json initialStats);
	static ECS::Entity createEmber(vec2 position);
};