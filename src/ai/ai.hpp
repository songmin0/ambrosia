#pragma once
#include "game/common.hpp"
#include "game/events.hpp"
#include "game/event_system.hpp"
#include "entities/tiny_ecs.hpp"
#include "maps/path_finding_system.hpp"

#include <vector>

// A simple AI system that moves the enemies (mobs and bosses)
class AISystem
{
public:
	AISystem(const PathFindingSystem& pfs);
	~AISystem();

	void step(float elapsed_ms, vec2 window_size_in_game_units);

	// Holds information
	struct MobComponent
	{
		ECS::Entity closestPlayer;	// the closest player to mob
		ECS::Entity getClosestPlayer();
		void setClosestPlayer(ECS::Entity);
	};

private:
	bool getClosestPlayer(ECS::Entity& mob);

	void startMobMovement(ECS::Entity entity);
	void startMobSkill(ECS::Entity entity);

	void onStartMobMovementEvent(const StartMobMovementEvent& event);
	void onStartMobSkillEvent(const StartMobSkillEvent& event);

	EventListenerInfo startMobMovementListener;
	EventListenerInfo startMobSkillListener;

	const PathFindingSystem& pathFindingSystem;
};
