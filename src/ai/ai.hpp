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
		ECS::Entity closestPlayer;	// the closest player to mob
		ECS::Entity getClosestPlayer();
		void setClosestPlayer(ECS::Entity);
	};

private:
	bool getClosestPlayer(ECS::Entity& mob);

	void startMobMoveCloser(ECS::Entity entity);
	void startMobRunAway(ECS::Entity entity);
	void startMobSkill(ECS::Entity entity);

	void onStartMobMoveCloserEvent(const StartMobMoveCloserEvent& event);
	void onStartMobRunAwayEvent(const StartMobRunAwayEvent& event);
	void onStartMobSkillEvent(const StartMobSkillEvent& event);

	EventListenerInfo startMobMoveCloserListener;
	EventListenerInfo startMobRunAwayListener;
	EventListenerInfo startMobSkillListener;

	PathFindingSystem& pathFindingSystem;
};
