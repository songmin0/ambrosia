#include "swarm_behaviour.hpp"
#include "entities/tiny_ecs.hpp"
#include "entities/enemies.hpp"
#include "maps/map.hpp"
#include "game/stats_system.hpp"

#include <math.h>
#include <iostream>
#include <maps/path_finding_system.hpp>

using namespace std;

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
	PathFindingSystem pathFindingSystem;

	for (int y = 0; y < map.grid.size(); y++) {
		for (int x = 0; x < map.grid[0].size(); x++) {
			if (map.grid[y][x] == 3 && pathFindingSystem.isWalkablePoint(getWorldPosition(vec2(x, y), map))) {
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
	PathFindingSystem pathFindingSystem;

	for (int i = 0; i < totalPoints; i++) {
		// get ith angle around centre
		float angle = theta * i;
		float x = radius * cos(angle) + centre.x;
		float y = radius * sin(angle) + centre.y;

		// check validity of point
		if (x < 0 || y < 0 || y > map.grid.size() || x > map.grid[0].size()
			|| map.grid[y / map.tileSize][x / map.tileSize] == 0 ||
			!pathFindingSystem.isWalkablePoint(vec2(x, y))) {
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
	auto potato_pos = ECS::registry<Motion>.get(potato).position;
	auto map = ECS::registry<MapComponent>.components.front();
	auto points = getPointsAroundCentre(200, potato_pos, num_chunks);

	for (int i = 0; i < num_chunks; i++) {
		PotatoChunk::createPotatoChunk(points[i], potato_pos, -1);
	}
}

void SwarmBehaviour::startWait(ECS::Entity potato) {
	waitingForPotatoToExplode = 500;
	curr_potato = potato;
}

void SwarmBehaviour::step(float elapsed_ms, vec2 window_size_in_game_units) {

	auto chunks = ECS::registry<ActivePotatoChunks>.entities;

	// chunks exist, and arent currently dying
	if (chunks.size() > 0 && !ECS::registry<DeathTimer>.has(chunks[0])) {
		auto potato = ECS::registry<ActivePotatoChunks>.get(chunks[0]).potato;
		auto potato_pos = ECS::registry<Motion>.get(potato).position;

		// check if all chunks are within some distance d from potato;
		for (auto chunk : chunks) {
			auto chunk_pos = ECS::registry<Motion>.get(chunk).position;
			// if not, return
			if (!(chunk_pos.x < potato_pos.x + 50 &&
				chunk_pos.y < potato_pos.y + 50 &&
				chunk_pos.x > potato_pos.x - 50 &&
				chunk_pos.y > potato_pos.y - 50)) {
				return;
			}
		}
		auto max_hp = chunks[0].get<StatsComponent>().getStatValue(StatType::MAX_HP) * num_chunks;
		auto remaining_hp = 0.f;
		for (auto chunk : chunks) {
			remaining_hp += chunk.get<StatsComponent>().getStatValue(StatType::HP);
			ECS::registry<DeathTimer>.emplace(chunk).CustomDeathTimer(100.f);
		}

		auto mashed_potato_hp = remaining_hp / max_hp;
		std::cout << "Spawning mashed potato";
		MashedPotato::createMashedPotato(potato_pos, mashed_potato_hp);
	}
}

ActivePotatoChunks::ActivePotatoChunks(ECS::Entity potato) {
	this->potato = potato;
}