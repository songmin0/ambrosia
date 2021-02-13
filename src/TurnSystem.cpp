#include "TurnSystem.hpp"

#include <string.h>
#include <cassert>
#include <sstream>
#include <iostream>

#include "ai.hpp"
#include "render_components.hpp"

TurnSystem::TurnSystem(const PathFindingSystem& pfs)
		: pathFindingSystem(pfs)
{
		EventSystem<MouseClickEvent>::Instance().RegisterListener(
				std::bind(&TurnSystem::OnMouseClick, this, std::placeholders::_1));
}

TurnSystem::~TurnSystem()
{
		if (mouseClickListener.IsValid())
		{
				EventSystem<MouseClickEvent>::Instance().UnregisterListener(mouseClickListener);
		}
}

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
						ECS::registry<TurnComponentIsActive>.emplace(entity);
						break;
				}
		}

		//if all the players have gone start going through all the mobs
		if (ECS::registry<TurnComponentIsActive>.size() == 0) {
				auto& registry = ECS::registry<AISystem::MobComponent>;
				for (unsigned int i = 0; i < registry.components.size(); i++) {
						ECS::Entity entity = registry.entities[i];
						auto& turnComponent = ECS::registry<TurnComponent>.get(entity);
						if (!turnComponent.hasGone) {
								ECS::registry<TurnComponentIsActive>.emplace(entity);
								break;
						}
				}
		}

		if (ECS::registry<TurnComponentIsActive>.size() != 0) {
				//Set the activeEnity's hasGone to true
				// Check happens with if statement
				auto& activeEntity = ECS::registry<TurnComponentIsActive>.entities[0];
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
		assert(nextEntity.has<TurnComponent>());
		auto& turnComponent = ECS::registry<TurnComponent>.get(nextEntity);
		if (!turnComponent.hasGone) {
				assert(ECS::registry<TurnComponentIsActive>.entities.size() > 0);
				ECS::registry<TurnComponent>.get(ECS::registry<TurnComponentIsActive>.entities[0]).hasGone = false;
				std::cout << "switching to the next active entity \n";
				//Remove the active entity
				ECS::registry<TurnComponentIsActive>.clear();

				//Set the activeEntity to the nextEntity
				ECS::registry<TurnComponentIsActive>.emplace(nextEntity);

				//Set the activeEnity's hasGone to true
				auto& activeEntityTurnComponent = ECS::registry<TurnComponent>.get(nextEntity);
				activeEntityTurnComponent.hasGone = true;
		}
		else {
				assert(ECS::registry<TurnComponentIsActive>.entities.size() > 1);
				if (nextEntity.id == ECS::registry<TurnComponentIsActive>.entities[0].id){
						std::cout << "The requested player is already the active player\n";
				}
				else {
						std::cout << "player has already gone this turn\n";
				}
		}
		

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
		//If there is no active entity (this could be due to a restart) get the next active entity
		if (ECS::registry<TurnComponentIsActive>.entities.size() == 0) {
				nextActiveEntity();
		}
		// Check happens with above if statement.
		auto& activeEntity = ECS::registry<TurnComponentIsActive>.entities[0];
		if (!activeEntity.has<DeathTimer>()) {
				//TODO this should check both that the player has moved and used a skill so update once we have skills
				if (ECS::registry<TurnComponent>.get(activeEntity).hasMoved) {
						nextActiveEntity();
				}
		}
		else {	
				//Pretty sure whis will be needed once we get multiple players but that depends 
				//		on how we handle death leaving for now as it doesn't hurt anything
				//The current user is dead so switch to the next
				nextActiveEntity();
		}
}

void TurnSystem::OnMouseClick(const MouseClickEvent &event)
{
	// This is probably just temporary code. It makes the player move when you click on the screen (unless
	// the player is already moving)

	if (!ECS::registry<TurnComponentIsActive>.entities.empty())
	{
		// Get the active entity
		// Check already occurs with if statement above
		auto& activeEntity = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];

		// Check that it's a player and has a Motion component
		if (activeEntity.has<PlayerComponent>() && activeEntity.has<Motion>())
		{
			auto& motion = activeEntity.get<Motion>();

			// If the entity is not currently moving, create a path so that it will start moving
			if (motion.path.empty())
			{
				motion.path = pathFindingSystem.GetShortestPath(motion.position, event.mousePos);



				// TEMPORARY: If you click on an unwalkable area of the map, the character will do a bone throw
				if(motion.path.empty())
				{
					LaunchBoneEvent launchBoneEvent;
					launchBoneEvent.instigator = activeEntity;
					launchBoneEvent.targetPosition = event.mousePos;
					EventSystem<LaunchBoneEvent>::Instance().SendEvent(launchBoneEvent);
				}
			}
		}
	}
}
