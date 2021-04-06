#pragma once
#include "game/common.hpp"
#include "entities/tiny_ecs.hpp"

struct CheeseBlob
{
	static ECS::Entity createCheeseBlob(vec2 position);
};

struct DessertForeground
{
	static ECS::Entity createDessertForeground(vec2 position);
};

struct BBQBackground { static ECS::Entity createBBQBackground(vec2 position); };