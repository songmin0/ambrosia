#pragma once
#include "tiny_ecs.hpp"
#include "game/common.hpp"

struct Taji
{
public:
	static ECS::Entity createTaji(json initialStats);
	static ECS::Entity createTaji(vec2 pos);
private:
	static ECS::Entity commonInit();
};
