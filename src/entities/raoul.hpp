#pragma once
#include "tiny_ecs.hpp"
#include "game/common.hpp"

struct Raoul
{
public:
	static ECS::Entity createRaoul(json initialStats);
	static ECS::Entity createRaoul(vec2 position);
private:
	static ECS::Entity commonInit();
};
