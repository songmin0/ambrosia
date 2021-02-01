#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"

struct Raoul
{
	// Creates all the associated render resources and default transform
	static ECS::Entity CreateRaoul(vec2 pos);
};
