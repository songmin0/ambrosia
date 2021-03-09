#pragma once
#include "tiny_ecs.hpp"
#include "game/common.hpp"

struct Pepper
{
	static ECS::Entity createPepper(vec2 pos);
};

struct Potato
{
	static ECS::Entity createPotato(vec2 pos);
};

