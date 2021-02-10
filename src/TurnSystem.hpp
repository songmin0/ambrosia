#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "functional"
#include "PathFindingSystem.hpp"
#include "Events.hpp"
#include "EventSystem.hpp"


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
	void OnMouseClick(const MouseClickEvent& event);

	EventListenerInfo mouseClickListener;
	const PathFindingSystem& pathFindingSystem;
};
