#pragma once
#include "entities/tiny_ecs.hpp"
#include "game/common.hpp"

class SwarmBehaviour
{
	public:
		void spawnExplodedChunks(ECS::Entity potato);
		void startWait(ECS::Entity potato);
		void step(float elapsed_ms, vec2 window_size_in_game_units);
	private:
		int waitingForPotatoToExplode;
		bool spawnedBoss;
		ECS::Entity curr_potato;
		int num_chunks = 5;
};


struct HasSwarmBehaviour
{

};

struct ActivePotatoChunks
{
	// the potato it belongs to
	ECS::Entity potato;
	ActivePotatoChunks(ECS::Entity potato);
};



