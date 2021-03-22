#pragma once
#include "tiny_ecs.hpp"
#include "game/common.hpp"

struct Egg
{
	// Creates all the associated render resources and default transform
	static ECS::Entity createEgg(json stats, json position);
};

struct Pepper
{
	static ECS::Entity createPepper(json stats, json position);
};

struct Milk
{
	static ECS::Entity createMilk(json stats, json position);
};

struct Potato
{
	static ECS::Entity createPotato(json stats, json position);
};

struct MashedPotato
{
	static ECS::Entity createMashedPotato(vec2 pos, float initHPPercent = 1.f, float orientation = -1.f);
};

struct PotatoChunk
{
	static ECS::Entity createPotatoChunk(vec2 pos, vec2 potato_pos, float orientation = 1.f);
};

void createEnemies(json mobsConfig);
