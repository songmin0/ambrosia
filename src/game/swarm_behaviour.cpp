#include "swarm_behaviour.hpp"
#include <entities/tiny_ecs.hpp>
#include "entities/enemies.hpp"
#include "maps/map.hpp"
#include <math.h>
#include <iostream>
#define _USE_MATH_DEFINES
#include <cmath>
using namespace std;

// Takes a world position and converts it to the position of a tile in the grid
vec2 getGridPosition(vec2 worldPosition, MapComponent map) {
	return floor(worldPosition / map.tileSize);
}
// Takes the position of a tile in the grid and returns the world position on the actual map
vec2 getWorldPosition(vec2 gridPosition, MapComponent map) {
	return gridPosition * map.tileSize;
}
// simple way of getting the closest point in the grid
vec2 getClosestValidPoint(vec2 point) {
	auto map = ECS::registry<MapComponent>.components.front();
	vec2 grid_point = vec2(floor(point.x / map.tileSize), floor(point.y / map.tileSize));

	float min_displacement = INT_MAX;
	vec2 min_coords;

	for (int y = 0; y < map.grid.size(); y++) {
		for (int x = 0; x < map.grid[0].size(); x++) {
			if (map.grid[y][x] == 3) {
				vec2 disp_vectors = vec2(x, y) - grid_point;
				float displacement = sqrt(pow(disp_vectors.x, 2) + pow(disp_vectors.y, 2));
				if (min_displacement > displacement) {
					min_displacement = displacement;
					min_coords = vec2(x, y);
				}
			}
		}
	}

	return getWorldPosition(min_coords, map);
}

std::vector<vec2> getPointsAroundCentre(int radius, vec2 centre, int totalPoints) {
	float theta = 6.28318530718 / totalPoints;
	auto map = ECS::registry<MapComponent>.components.front();

	vector<vec2> res;

	for (int i = 0; i < totalPoints; i++) {
		// get ith angle around centre
		float angle = theta * i;
		float x = radius * cos(angle) + centre.x;
		float y = radius * sin(angle) + centre.y;
		
		// check validity of point
		if (x < 0 || y < 0 || y > map.grid.size() || x > map.grid[0].size() 
			|| map.grid[y / map.tileSize][x / map.tileSize] == 0) {
			res.push_back(getClosestValidPoint(vec2(x, y)));
		}
		else {
			res.push_back(vec2(x, y));
		}
	}
	return res;
}

// Each chunk belongs to a potato
// spawn chunks evenly spaced
// if not valid, spawn on point closest to it
void SwarmBehaviour::spawnExplodedChunks(ECS::Entity potato)
{
	std::cout << "Spawning potato chunks";
	vec2 potato_pos = ECS::registry<Motion>.get(potato).position;

	// make 5 chunks
	int num_chunks = 5;
	auto map = ECS::registry<MapComponent>.components.front();
	auto points = getPointsAroundCentre(300, potato_pos, num_chunks);

	for (int i = 0; i < num_chunks; i++) {
		PotatoChunk::createPotatoChunk(points[i], potato, -1);
	}
}

void SwarmBehaviour::step(float elapsed_ms, vec2 window_size_in_game_units) {

	// monitor distance here
	
}

ActivePotatoChunks::ActivePotatoChunks(ECS::Entity potato) {
	this->potato = potato;
}