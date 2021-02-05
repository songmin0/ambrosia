#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"

struct Egg
{
	// Creates all the associated render resources and default transform
	static ECS::Entity CreateEgg(vec2 pos);
};
