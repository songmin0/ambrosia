// internal
#include "ai.hpp"
#include "tiny_ecs.hpp"
#include "physics.hpp"
#include "TurnSystem.hpp"

#include <iostream>

void AISystem::step(float elapsed_ms, vec2 window_size_in_game_units)
{
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A2: HANDLE FISH AI HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
	// You will likely want to write new functions and need to create
	// new data structures to implement a more sophisticated Fish AI. 
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	/*auto& players = ECS::registry<PlayerComponent>;
	for (unsigned int i = 0; i < players.components.size(); i++) {
		ECS::Entity entity = players.entities[i];
		auto& turnComponent = ECS::registry<TurnSystem::TurnComponent>.get(entity);
		if (!turnComponent.hasGone) {

		}
	}*/
	
	(void)elapsed_ms; // placeholder to silence unused warning until implemented
	(void)window_size_in_game_units; // placeholder to silence unused warning until implemented
}

ECS::Entity AISystem::MobComponent::GetTargetEntity()
{
	return this->target;
}

void AISystem::MobComponent::SetTargetEntity(ECS::Entity target)
{
	this->target = target;
	std::cout << "Mob target set\n";
}
