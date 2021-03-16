#pragma once
#include "game/common.hpp"
#include "game/events.hpp"
#include "game/event_system.hpp"
#include "entities/tiny_ecs.hpp"
#include "maps/path_finding_system.hpp"
#include "ai/behaviour_tree.hpp"

#include <vector>

// A simple AI system that moves the enemies (mobs and bosses)
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
	bool setTargetToAllyMob(ECS::Entity& mob);

	void startMobMoveToPlayer(ECS::Entity entity);
	void startMobMoveToMob(ECS::Entity entity);
	void startMobRunAway(ECS::Entity entity);
	void startMobSkill(ECS::Entity entity);

	void onStartMobMoveToPlayerEvent(const StartMobMoveToPlayerEvent& event);
	void onStartMobMoveToMobEvent(const StartMobMoveToMobEvent& event);
	void onStartMobRunAwayEvent(const StartMobRunAwayEvent& event);
	void onStartMobSkillEvent(const StartMobSkillEvent& event);

	EventListenerInfo startMobMoveToPlayerListener;
	EventListenerInfo startMobMoveToMobListener;
	EventListenerInfo startMobRunAwayListener;
	EventListenerInfo startMobSkillListener;

	PathFindingSystem& pathFindingSystem;
};
