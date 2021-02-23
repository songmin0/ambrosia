#pragma once
#include "common.hpp"
#include "event_system.hpp"
#include "events.hpp"

#include "entities/tiny_ecs.hpp"
#include "maps/path_finding_system.hpp"

#include <functional>

// A system to handle turns
class TurnSystem
{

public:
		TurnSystem(const PathFindingSystem& pfs);
		~TurnSystem();

		void nextActiveEntity();
		static void changeActiveEntity(ECS::Entity nextEntity);
		void nextTurn();

		void step(float elapsed_ms);
		

		// Stucture to store turn information
		struct TurnComponent
		{
			inline bool canStartMoving() {return !hasMoved && !isMoving;}
			inline bool canStartSkill() {return hasMoved && !hasUsedSkill && !isUsingSkill;}

			bool isMoving = false;
			bool isUsingSkill = false;

			bool hasMoved = false;
			bool hasUsedSkill = false;
		};

		struct TurnComponentIsActive
		{

		};

		static bool hasCompletedTurn(TurnComponent tc);

private:
	const PathFindingSystem& pathFindingSystem;

	void onMouseClick(const MouseClickEvent& event);
	EventListenerInfo mouseClickListener;

	void onPlayerButtonClick(const PlayerButtonEvent& event);
	EventListenerInfo playerButtonListener;

	void onFinishedMovement(const FinishedMovementEvent& event);
	EventListenerInfo finishedMovementListener;

	void onFinishedSkill(const FinishedSkillEvent& event);
	EventListenerInfo finishedSkillListener;
};
