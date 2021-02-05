#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "functional"


// A system to handle turns
class TurnSystem
{
private:
		//std::vector<std::function<void()>> callbacks;
		ECS::Entity activeEntity;
		
		

public:
		
		ECS::Entity getActiveEntity();
		//void attach(std::function<void()> update);
		void nextActiveEntity();
		void changeActiveEntity(ECS::Entity nextEntity);
		void nextTurn();

		void step(float elapsed_ms);
		

		// Stucture to store turn information
		struct TurnComponent
		{
				//bool isActive;
				bool hasGone = false;
				bool hasMoved = false;
				bool hasUsedSKill = false;
		};

		struct TurnComponentIsActive
		{

		};
};
