#pragma once

#include "common.hpp"
#include "Map.hpp"

class PathFindingSystem
{
public:
	PathFindingSystem() = default;
	~PathFindingSystem() = default;

	// Uses BFS to find the shortest path from source to destination in a grid
	std::stack<vec2> GetShortestPath(vec2 source, vec2 destination) const;

private:
	// Checks that the point is within the bounds of the map
	bool IsValidPoint(const MapComponent& map, vec2 point) const;

	// Checks that there is no obstacle at the given point
	bool IsWalkablePoint(const MapComponent& map, vec2 point) const;

	// After the "visited" matrix has been filled, this function helps in finding the
	// shortest path from destination back to source
	vec2 GetCheapestAdjacentPoint(const MapComponent& map,
															 	const std::vector<std::vector<int>>& distance,
															 	std::vector<std::vector<bool>>& visited,
															 	vec2 p) const;

	// If the destination point is too close to the source, no path will be generated
	const float MIN_DISTANCE_SRC_TO_DST = 5.f;
};

