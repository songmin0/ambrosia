// internal
#include "ai.hpp"
#include "tiny_ecs.hpp"
#include "physics.hpp"

void AISystem::step(float elapsed_ms, vec2 window_size_in_game_units)
{
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A2: HANDLE FISH AI HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
	// You will likely want to write new functions and need to create
	// new data structures to implement a more sophisticated Fish AI. 
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// TODO M1: REQUIRE PLAYER COMPOENENT IN TURN SYSTEM
	// auto& motion = ECS::registry<Player>.components;
	(void)elapsed_ms; // placeholder to silence unused warning until implemented
	(void)window_size_in_game_units; // placeholder to silence unused warning until implemented
}

void AISystem::Mob::TargetEntity(ECS::Entity& target)
{
	this->target = target;
}
