#pragma once
#include "game/common.hpp"
#include "game/events.hpp"
#include "game/event_system.hpp"
#include "entities/tiny_ecs.hpp"
#include "maps/path_finding_system.hpp"
#include "ai/behaviour_tree.hpp"

#include <vector>

// AI system that moves the enemies (mobs and bosses)
class AISystem
{
public:
	AISystem(PathFindingSystem& pfs);
	~AISystem();

	void step(float elapsed_ms, vec2 window_size_in_game_units);

	// Holds information
	struct MobComponent
	{
		ECS::Entity target;
		ECS::Entity getTarget();
		void setTarget(ECS::Entity);
	};

private:
	bool setTargetToClosestPlayer(ECS::Entity& mob);
	bool setTargetToFarthestPlayer(ECS::Entity& mob);
	bool setTargetToWeakestPlayer(ECS::Entity& mob);
	bool setTargetToWeakestMob(ECS::Entity& mob);
	bool setTargetToDeadPotato(ECS::Entity& mob);

	void startMobMove(ECS::Entity entity, MovementType movement);
	void startMobSkill(ECS::Entity entity);

	void onStartMobMoveEvent(const StartMobMoveEvent& event);
	void onStartMobSkillEvent(const StartMobSkillEvent& event);

	EventListenerInfo startMobMoveListener;
	EventListenerInfo startMobSkillListener;

	PathFindingSystem& pathFindingSystem;
};
