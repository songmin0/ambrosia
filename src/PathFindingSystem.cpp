#include "PathFindingSystem.hpp"

#include <queue>

std::stack<vec2> PathFindingSystem::GetShortestPath(vec2 source, vec2 destination) const
{
	// This will be the final result
	std::stack<vec2> shortestPath;

	// Make sure that the map exists
	if (ECS::registry<MapComponent>.components.empty())
	{
		return shortestPath;
	}

	// For now, just take the first/only map in the registry
	const MapComponent& map = ECS::registry<MapComponent>.components.front();

	// Should always have a valid map
	assert(!map.grid.empty());

	// Convert the source and destination points to grid coordinates
	vec2 gridSource = round(source / map.tileSize);
	vec2 gridDestination = round(destination / map.tileSize);

	// If the destination is not on the map, not walkable, or too close to the source, return an empty path
	if (!IsValidPoint(map, gridDestination) || !IsWalkablePoint(map, gridDestination) || gridSource == gridDestination)
	{
		return shortestPath;
	}

	// The current data structure of our pizza-arena map is a 2D array of size 40x32. Each element in the array represents
	// one tile, and the tile size is 32. When doing pathfinding, I'm using the full size of the map. This is an easy way
	// to do it because each position on the screen corresponds to a position in these vectors:
	int gridWidth = map.grid[0].size();
	int gridHeight = map.grid.size();

	std::vector<std::vector<bool>> visited(gridHeight, std::vector<bool>(gridWidth));
	std::vector<std::vector<int>> distance(gridHeight, std::vector<int>(gridWidth));

	// Start at the source point
	std::queue<vec2> queue;
	queue.push(gridSource);

	visited[gridSource.y][gridSource.x] = true;
	distance[gridSource.y][gridSource.x] = 0;

	// Keep searching until destination is reached or the queue is empty (i.e., no possible way to reach destination)
	while (!queue.empty())
	{
		// Take the first point from the queue. We'll look at its neighbours and add any valid ones to the queue
		vec2 currentPoint = queue.front();
		queue.pop();

		// If the destination is reached, the search is over
		if (currentPoint == gridDestination)
		{
			break;
		}

		// A lambda to cut down on code duplication when checking the current point's neighbours
		auto checkPoint = [&](vec2 adjacentPoint) {
			if (IsValidPoint(map, adjacentPoint) && !visited[adjacentPoint.y][adjacentPoint.x])
			{
				// Set visited so that we never check this point again
				visited[adjacentPoint.y][adjacentPoint.x] = true;

				// If it's a walkable point, set its distance to be 1 greater than its "parent", and then add it to the queue
				if (IsWalkablePoint(map, adjacentPoint))
				{
					distance[adjacentPoint.y][adjacentPoint.x] = distance[currentPoint.y][currentPoint.x] + 1;
					queue.push(adjacentPoint);
				}
					// Otherwise, invalidate the distance, just to be safe
				else
				{
					distance[adjacentPoint.y][adjacentPoint.x] = INT_MAX;
				}
			}
		};

		// Process each of the neighbours
		checkPoint(vec2(currentPoint.x - 1, currentPoint.y));
		checkPoint(vec2(currentPoint.x + 1, currentPoint.y));
		checkPoint(vec2(currentPoint.x, currentPoint.y - 1));
		checkPoint(vec2(currentPoint.x, currentPoint.y + 1));
	}

	// If destination was visited, then a path exists. Go backward (from destination to source) to find shortest path
	if (visited[gridDestination.y][gridDestination.x])
	{
		// Add the actual destination to the path
		shortestPath.push(destination);

		// Start working backward from the best neighbouring point
		vec2 currentPoint = GetCheapestAdjacentPoint(map, distance, visited, gridDestination);

		// Keep going until we're back at the source
		while (currentPoint != gridSource)
		{
			// Add the current point to the path, putting it back into the proper coordinate system
			shortestPath.push(currentPoint * map.tileSize);

			// Get the neighbour with the shortest distance to the source
			currentPoint = GetCheapestAdjacentPoint(map, distance, visited, currentPoint);
		}

		// Add the source point to the path
		shortestPath.push(currentPoint * map.tileSize);
	}

	return shortestPath;
}

bool PathFindingSystem::IsValidPoint(const MapComponent& map, vec2 point) const
{
	// Check that the point is within the bounds of the map
	return point.x >= 0 && point.x < map.grid[0].size()
					&& point.y >= 0 && point.y < map.grid.size();
}

bool PathFindingSystem::IsWalkablePoint(const MapComponent& map, vec2 point) const
{
	// Check that the point is marked as walkable
	return map.grid[point.y][point.x] == 3;
}

vec2 PathFindingSystem::GetCheapestAdjacentPoint(const MapComponent& map,
																								 const std::vector<std::vector<int>>& distance,
																								 std::vector<std::vector<bool>>& visited,
																								 vec2 point) const
{
	// We'll put the point's neighbours in this list and then see which one has the shortest path back to the source
	std::vector<vec2> validAdjacentPoints;

	// A lambda to cut down on code duplication when checking the adjacent points
	auto checkPoint = [&](vec2 p) {
		if (IsValidPoint(map, p))
		{
			// If the point is walkable and we visited it, then we can consider it
			if (IsWalkablePoint(map, p) && visited[p.y][p.x])
			{
				validAdjacentPoints.push_back(p);
			}

			// Make sure we don't consider this point again
			visited[p.y][p.x] = false;
		}
	};

	// Check all four neighbours. If it's a walkable point that we have visited, add it to the list for consideration.
	// After this, we'll determine which one is closest to the source point
	checkPoint(vec2(point.x - 1, point.y));
	checkPoint(vec2(point.x + 1, point.y));
	checkPoint(vec2(point.x, point.y - 1));
	checkPoint(vec2(point.x, point.y + 1));

	// If there are no valid adjacent points, then there's a problem in PathFindingSystem::GetShortestPath and this
	// function should never have been called. It only works if we already know that there's a valid path from source
	// to destination.
	assert(!validAdjacentPoints.empty());

	// Determine which neighbour has the shortest path back to the source
	vec2 bestPoint = validAdjacentPoints.front();
	for (int i = 1; i < validAdjacentPoints.size(); i++)
	{
		vec2 currPoint = validAdjacentPoints.at(i);

		if (distance[currPoint.y][currPoint.x] < distance[bestPoint.y][bestPoint.x])
		{
			bestPoint = currPoint;
		}
	}

	// Make sure that we don't visit this point again
	visited[point.y][point.x] = false;

	return bestPoint;
}
