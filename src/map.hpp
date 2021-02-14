#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"

struct MapComponent
{
	// Creates all the associated render resources
	static ECS::Entity createMap(const std::string& name, vec2 screenSize);

	std::string name;
	float tileSize;
	std::vector<std::vector<int>> grid;
};