#pragma once
#include "tiny_ecs.hpp"
#include "game/common.hpp"

struct Egg
{
	// Creates all the associated render resources and default transform
	static ECS::Entity createEgg(vec2 pos);
};

struct Pepper
{
	static ECS::Entity createPepper(vec2 pos);
};

struct Potato
{
	static ECS::Entity createPotato(vec2 pos);
};

