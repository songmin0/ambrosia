#pragma once

#include "common.hpp"
#include "Map.hpp"

class PathFindingSystem
{
public:
	PathFindingSystem() = default;
	~PathFindingSystem() = default;

	// Uses BFS to find the shortest path from source to destination in a grid
	std::stack<vec2> getShortestPath(vec2 source, vec2 destination) const;

private:
	// Checks that the point is within the bounds of the map
	bool isValidPoint(const MapComponent& map, vec2 point) const;

	// Checks that there is no obstacle at the given point
	bool isWalkablePoint(const MapComponent& map, vec2 point) const;

	// After the "visited" matrix has been filled, this function helps in finding the
	// shortest path from destination back to source
	vec2 getCheapestAdjacentPoint(const MapComponent& map,
															 	const std::vector<std::vector<int>>& distance,
															 	std::vector<std::vector<bool>>& visited,
															 	vec2 p) const;
};

