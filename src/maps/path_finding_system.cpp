#include "path_finding_system.hpp"
#include "ai/ai.hpp"

#include <queue>

std::stack<vec2> PathFindingSystem::getShortestPath(ECS::Entity sourceEntity, vec2 destination)
{
	// This will be the final result
	std::stack<vec2> shortestPath;

	// It would be a bug if we don't have exactly one map loaded
	assert(ECS::registry<MapComponent>.components.size() == 1);

	// It would be a bug if the current map has an empty grid
	assert(!ECS::registry<MapComponent>.components.front().grid.empty());

	assert(sourceEntity.has<Motion>());
	vec2 source = sourceEntity.get<Motion>().position;

	setCurrentObstacles(sourceEntity);

	// Convert the source and destination points to grid coordinates
	vec2 gridSource = getGridPosition(source);
	vec2 gridDestination = getGridPosition(destination);

	// Not totally needed, but I'm just grabbing a reference to the map here
	const MapComponent& map = getMap();

	// If the destination is too close to the source, return an empty path
	if (gridSource == gridDestination || !isWalkablePoint(map, gridSource))
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

	// If it's not possible to walk all the way to the destination, then it navigates to the walkable tile which is
	// closest to the source.
	constexpr auto float_max = std::numeric_limits<float>::max();
	vec2 closestTile = gridSource;
	float closestDistance = float_max;

	// Keep searching until destination is reached or the queue is empty (i.e., no possible way to reach destination)
	while (!queue.empty())
	{
		// Take the first point from the queue. We'll look at its neighbours and add any valid ones to the queue
		// Check happens within while statement
		vec2 currentPoint = queue.front();
		queue.pop();

		// If the destination is reached, the search is over
		if (currentPoint == gridDestination)
		{
			break;
		}

		// A lambda to cut down on code duplication when checking the current point's neighbours
		auto checkPoint = [&](vec2 adjacentPoint) {
			if (isValidPoint(map, adjacentPoint) && !visited[adjacentPoint.y][adjacentPoint.x])
			{
				// Set visited so that we never check this point again
				visited[adjacentPoint.y][adjacentPoint.x] = true;

				// If it's a walkable point, set its distance to be 1 greater than its "parent", and then add it to the queue
				if (isWalkablePoint(map, adjacentPoint))
				{
					distance[adjacentPoint.y][adjacentPoint.x] = distance[currentPoint.y][currentPoint.x] + 1;
					queue.push(adjacentPoint);

					float distanceToDest = glm::distance(adjacentPoint, gridDestination);
					if (distanceToDest < closestDistance)
					{
						closestDistance = distanceToDest;
						closestTile = adjacentPoint;
					}
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

	// Go backward from the point that was reached (which might be the destination or might be the closest point to the
	// destination) and find the shortest path back to the source
	if (closestDistance < float_max)
	{
		// Add the destination tile as the last point in the path
		shortestPath.push(getWorldPosition(closestTile));

		// Start working backward from the best neighbouring point
		vec2 currentPoint = getCheapestAdjacentPoint(map, distance, visited, closestTile);

		// Keep going until we're back at the source
		while (currentPoint != gridSource)
		{
			// Add the current point to the path, putting it back into the proper coordinate system
			shortestPath.push(getWorldPosition(currentPoint));

			// Get the neighbour with the shortest distance to the source
			currentPoint = getCheapestAdjacentPoint(map, distance, visited, currentPoint);
		}

		// Add the source point to the path
		shortestPath.push(getWorldPosition(currentPoint));
	}

	return shortestPath;
}
bool PathFindingSystem::isWalkablePoint(vec2 point)
{
	assert(ECS::registry<MapComponent>.components.size() == 1);
	assert(!ECS::registry<MapComponent>.components.front().grid.empty());

	// Populate the list of obstacles
	setCurrentObstacles();

	// Check the given point
	const MapComponent& map = getMap();
	vec2 gridPosition = getGridPosition(point);
	return isValidPoint(map, gridPosition) && isWalkablePoint(map, gridPosition);
}

bool PathFindingSystem::isWalkablePoint(ECS::Entity entity, vec2 point)
{
	assert(ECS::registry<MapComponent>.components.size() == 1);
	assert(!ECS::registry<MapComponent>.components.front().grid.empty());

	// Populate the list of obstacles
	setCurrentObstacles(entity);

	// Check the given point
	const MapComponent& map = getMap();
	vec2 gridPosition = getGridPosition(point);
	return isValidPoint(map, gridPosition) && isWalkablePoint(map, gridPosition);
}

bool PathFindingSystem::isValidPoint(const MapComponent& map, vec2 point) const
{
	// Check that the point is within the bounds of the map
	return point.x >= 0 && point.x < map.grid[0].size()
					&& point.y >= 0 && point.y < map.grid.size();
}

bool PathFindingSystem::isWalkablePoint(const MapComponent& map, vec2 point) const
{
	// Check that the point is marked as walkable and there's no obstacle at that position
	if (point.y >= map.grid.size() || point.x >= map.grid[point.y].size())
	{
		std::cout << "WARNING: attempting to access a point outside of the map grid" << std::endl;
		return false;
	}

	return map.grid[point.y][point.x] == 3 &&
		std::find(obstacles.begin(), obstacles.end(), point) == obstacles.end();
}

vec2 PathFindingSystem::getCheapestAdjacentPoint(const MapComponent& map,
																								 const std::vector<std::vector<int>>& distance,
																								 std::vector<std::vector<bool>>& visited,
																								 vec2 point) const
{
	// We'll put the point's neighbours in this list and then see which one has the shortest path back to the source
	std::vector<vec2> validAdjacentPoints;

	// A lambda to cut down on code duplication when checking the adjacent points
	auto checkPoint = [&](vec2 p) {
		if (isValidPoint(map, p))
		{
			// If the point is walkable and we visited it, then we can consider it
			if (isWalkablePoint(map, p) && visited[p.y][p.x])
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
	// Checked in an assert above because this validAdjacentPoints should never be empty
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

void PathFindingSystem::setCurrentObstacles()
{
	obstacles.clear();

	for (auto entity : ECS::registry<Motion>.entities)
	{
		if (entity.has<PlayerComponent>() || entity.has<AISystem::MobComponent>())
		{
			// The entity whose path we are generating can't be an obstacle, and
			// dead entities can't be obstacles either
			if (entity.has<DeathTimer>())
			{
				continue;
			}

			if (entity.has<Motion>())
			{
				// Store this entity as an obstacle in the grid
				obstacles.push_back(getGridPosition(entity.get<Motion>().position));
			}
		}
	}
}

void PathFindingSystem::setCurrentObstacles(ECS::Entity sourceEntity)
{
	setCurrentObstacles();

	if (sourceEntity.has<Motion>())
	{
		vec2 sourcePos = sourceEntity.get<Motion>().position;
		auto it = std::find(obstacles.begin(), obstacles.end(), getGridPosition(sourcePos));

		if (it != obstacles.end())
		{
			obstacles.erase(it);
		}
	}
}
