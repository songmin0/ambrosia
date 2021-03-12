#pragma once
#include "tiny_ecs.hpp"
#include "game/common.hpp"

struct Egg
{
	// Creates all the associated render resources and default transform
	static ECS::Entity createEgg(vec2 pos, float orientation = -1.f);
};

struct Pepper
{
	static ECS::Entity createPepper(vec2 pos, float orientation = -1.f);
};

struct Milk
{
	static ECS::Entity createMilk(vec2 pos, float orientation = 1.f);
};

struct Potato
{
	static ECS::Entity createPotato(vec2 pos, float orientation = -1.f);
};

struct MashedPotato
{
	static ECS::Entity createMashedPotato(vec2 pos, float initHPPercent = 1.f, float orientation = -1.f);
};

struct PotatoChunk
{
	static ECS::Entity createPotatoChunk(vec2 pos, float orientation = 1.f);
};

