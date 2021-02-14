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
				bool hasGone = false;
				bool hasMoved = false;
				bool hasUsedSKill = false;
		};

		struct TurnComponentIsActive
		{

		};

private:
	void onMouseClick(const MouseClickEvent& event);

	EventListenerInfo mouseClickListener;
	const PathFindingSystem& pathFindingSystem;
};
