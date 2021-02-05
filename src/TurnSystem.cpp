#include "TurnSystem.hpp"

#include <string.h>
#include <cassert>
#include <sstream>
#include <iostream>

#include "ai.hpp"

ECS::Entity TurnSystem::getActiveEntity()
{
		return activeEntity;
}

//void TurnSystem::attach(std::function<void()> update)
//{
//}

//Just sets the next available entity as the current entity
void TurnSystem::nextActiveEntity()
{
		std::cout << "switching to the next active entity \n";
		//Remove the active entity
		ECS::registry<TurnComponentIsActive>.clear();

		//Loop through the player's TurnComponents and set the first player found that hasn't gone yet
		auto& registry = ECS::registry<PlayerComponent>;
		for (unsigned int i = 0; i < registry.components.size(); i++) {
				ECS::Entity entity = registry.entities[i];
				auto& turnComponent = ECS::registry<TurnComponent>.get(entity);
				if (!turnComponent.hasGone) {
						activeEntity = entity;
						ECS::registry<TurnComponentIsActive>.emplace(entity);
						break;
				}
		}

		//if all the players have gone start going through all the mobs
		if (ECS::registry<TurnComponentIsActive>.size() == 0) {
				auto& registry = ECS::registry<AISystem::Mob>;
				for (unsigned int i = 0; i < registry.components.size(); i++) {
						ECS::Entity entity = registry.entities[i];
						auto& turnComponent = ECS::registry<TurnComponent>.get(entity);
						if (!turnComponent.hasGone) {
								activeEntity = entity;
								ECS::registry<TurnComponentIsActive>.emplace(entity);
								break;
						}
				}
		}

		if (ECS::registry<TurnComponentIsActive>.size() != 0) {
				//Set the activeEnity's hasGone to true
				auto& activeEntityTurnComponent = ECS::registry<TurnComponent>.get(activeEntity);
				activeEntityTurnComponent.hasGone = true;
		}
		else {
				//All entities have gone so end the turn
				nextTurn();
		}
		return;
}

//Sets the current entity to the specified "nextEntity"
void TurnSystem::changeActiveEntity(ECS::Entity nextEntity)
{
		//Make sure the nextEntity has a TurnComponent


		//Set the activeEntity to the nextEntity
		//Set the activeEnity's isActive and hasGone to true
}

//All players and mobs have completed their turn. This makes sure the next turn starts properly
void TurnSystem::nextTurn()
{
		std::cout << "Starting the next turn \n";
		//Clear the register for TurnComponentIsActive
		ECS::registry<TurnComponentIsActive>.clear();
		//Loop through all TurnComponents and set all the attributes back to false
		auto& registry = ECS::registry<TurnComponent>;
		for (unsigned int i = 0; i < registry.components.size(); i++) {
				auto& turnComponent = registry.components[i];
				turnComponent.hasGone = false;
				turnComponent.hasMoved = false;
				turnComponent.hasUsedSKill = false;
		}

		//start the next turn
		nextActiveEntity();

}

void TurnSystem::step(float elapsed_ms)
{
		//std::cout << "active entity: " << activeEntity.id << "\n";
		if (ECS::registry<TurnComponent>.get(activeEntity).hasMoved) {
				std::cout << "entity has moved switch to next entity \n";
				nextActiveEntity();
		}
}
