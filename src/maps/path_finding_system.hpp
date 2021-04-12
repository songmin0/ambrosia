#pragma once
#include "game/common.hpp"
#include "maps/map.hpp"

class PathFindingSystem
{
public:
	PathFindingSystem() = default;
	~PathFindingSystem() = default;

	// Uses BFS to find the shortest path from source to destination in a grid
	std::stack<vec2> getShortestPath(ECS::Entity sourceEntity, vec2 destination);

	// Checks that the point is within the bounds of the map and that there is no
	// obstacle at the given point (so it's a bit different than the private
	// function of the same name)
	bool isWalkablePoint(vec2 point);
	bool isWalkablePoint(ECS::Entity entity, vec2 point);

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

	// Returns a reference to the current map. This function should only be called after the
	// getShortestPath function has checked that a valid map exists
	inline const MapComponent& getMap() const {return ECS::registry<MapComponent>.components.front();}

	// Takes a world position and converts it to the position of a tile in the grid
	inline vec2 getGridPosition(vec2 worldPosition) const {return round(worldPosition / getMap().tileSize);}

	// Takes the position of a tile in the grid and returns the world position on the actual map
	inline vec2 getWorldPosition(vec2 gridPosition) const {return gridPosition * getMap().tileSize;}


	// Updates the current obstacles before pathfinding runs. The sourceEntity is
	// the entity whose path is being generated
	void setCurrentObstacles();
	void setCurrentObstacles(ECS::Entity sourceEntity);

	// Player and mob positions (gets updated before pathfinding starts)
	std::vector<vec2> obstacles;
};

