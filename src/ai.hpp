#pragma once

#include <vector>

#include "common.hpp"
#include "tiny_ecs.hpp"

// A simple AI system that moves the enemies (mobs and bosses)
class AISystem
{
public:
	void step(float elapsed_ms, vec2 window_size_in_game_units);

	// Holds information
	struct MobComponent
	{
		ECS::Entity target = ECS::registry<PlayerComponent>.entities[0];	// the (player) target of the mob
		ECS::Entity GetTargetEntity();
		void SetTargetEntity(ECS::Entity);
	};
};
