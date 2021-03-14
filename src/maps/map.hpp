#pragma once
#include "game/common.hpp"
#include "entities/tiny_ecs.hpp"

struct MapComponent
{
	// Creates all the associated render resources
	static ECS::Entity createMap(const std::string& name, vec2 screenSize);

	std::string name;
	vec2 mapSize;
	float tileSize = 32.f;
	std::vector<std::vector<int>> grid;
};