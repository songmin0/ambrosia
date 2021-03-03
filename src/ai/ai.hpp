#pragma once
#include "game/common.hpp"
#include "game/events.hpp"
#include "game/event_system.hpp"
#include "entities/tiny_ecs.hpp"
#include "maps/path_finding_system.hpp"

#include <vector>

enum class MobType
{
	MOB,
	BOSS
};

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
		MobType type;
	};

private:
	bool getClosestPlayer(ECS::Entity& mob);

	void startMobTurn(ECS::Entity entity);
	void startMobMovement(ECS::Entity entity, Motion& motion, MobComponent& mobComponent);
	void startMobSkill(ECS::Entity entity, Motion& motion, MobComponent& mobComponent);

	void onStartMobTurnEvent(const StartMobTurnEvent& event);

	EventListenerInfo startMobTurnListener;

	const PathFindingSystem& pathFindingSystem;
};
