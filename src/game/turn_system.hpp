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
		TurnSystem(PathFindingSystem& pfs);
		~TurnSystem();

		void nextActiveEntity();
		static void changeActiveEntity(ECS::Entity nextEntity);
		void nextTurn();

		void step(float elapsed_ms);

		bool playersLeft();
		

		// Structure to store turn information
		struct TurnComponent
		{
			inline bool canStartMoving() {return !hasMoved && !isMoving && !isUsingSkill;}
			inline bool canStartSkill() {return !isMoving && !hasUsedSkill && !isUsingSkill;}

			bool isMoving = false;
			bool isUsingSkill = false;

			bool hasMoved = false;
			bool hasUsedSkill = false;

			// stores the state of the player's current active action
			// ie. what would occur during an on mouse event
			SkillType activeAction = SkillType::NONE;
		};

		struct TurnComponentIsActive {};

		static bool hasCompletedTurn(TurnComponent tc);

private:
	PathFindingSystem& pathFindingSystem;

	void onMouseClick(const MouseClickEvent& event);
	EventListenerInfo mouseClickListener;

	void onPlayerButtonClick(const PlayerButtonEvent& event);
	EventListenerInfo playerButtonListener;

	void onFinishedMovement(const FinishedMovementEvent& event);
	EventListenerInfo finishedMovementListener;

	void onFinishedSkill(const FinishedSkillEvent& event);
	EventListenerInfo finishedSkillListener;

	void onPrepForNextMap(const PrepForNextMapEvent& event);
	EventListenerInfo prepForNextMapListener;

	// This amount of time works well for allowing skills to finish completely
	// before switching to the next active entity
	static constexpr float TIMER_PERIOD = 1200.f;
	float timer;
};
