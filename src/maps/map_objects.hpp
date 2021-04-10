#pragma once
#include "game/common.hpp"
#include "entities/tiny_ecs.hpp"
#include "rendering/render.hpp"

struct CheeseBlob
{
	static ECS::Entity createCheeseBlob(vec2 position);
};

struct DessertForeground
{
	static ECS::Entity createDessertForeground(vec2 position);
};

struct DessertBackground
{
	static ECS::Entity createDessertBackground(vec2 position);
};

struct BBQBackground { static ECS::Entity createBBQBackground(vec2 position); };

struct BBQFire { static ECS::Entity createBBQFire(vec2 position, RenderLayer layer = RenderLayer::MAP_FOREGROUND, vec2 scale = vec2(1.f)); };

struct ParallaxComponent {
	ParallaxComponent(vec2 scrollRate) : scrollRate{ scrollRate } {}
	vec2 scrollRate = { 1.f, 1.f };
};