#pragma once
#include "game/common.hpp"
#include "entities/tiny_ecs.hpp"
#include "maps/path_finding_system.hpp"

#include <vector>

// A simple AI system that moves the enemies (mobs and bosses)
class AISystem
{
public:
	AISystem(const PathFindingSystem& pfs);
	void step(float elapsed_ms, vec2 window_size_in_game_units);

	// Holds information
	struct MobComponent
	{
		ECS::Entity target;	// the (player) target of the mob
		ECS::Entity getTargetEntity();
		void setTargetEntity(ECS::Entity);
	};

private:
	const PathFindingSystem& pathFindingSystem;
};
