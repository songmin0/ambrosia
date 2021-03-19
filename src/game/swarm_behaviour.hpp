#pragma once
#include <entities/tiny_ecs.hpp>
#include "common.hpp"

class SwarmBehaviour
{
	public:
		void spawnExplodedChunks(ECS::Entity potato);
		void startWait(ECS::Entity potato);
		void step(float elapsed_ms, vec2 window_size_in_game_units);
	private:
		int waitingForPotatoToExplode;
		ECS::Entity curr_potato;
};


struct HasSwarmBehaviour
{

};

struct ActivePotatoChunks
{
	// the potato it belongs to
	vec2 potato_pos;
	ActivePotatoChunks(vec2 potato_pos);
};


