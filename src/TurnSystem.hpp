#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "functional"


// A system to handle turns
class TurnSystem
{

public:

		void nextActiveEntity();
		void changeActiveEntity(ECS::Entity nextEntity);
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
};
