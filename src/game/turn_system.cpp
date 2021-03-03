#include "turn_system.hpp"

#include "ai/ai.hpp"
#include "rendering/render_components.hpp"

#include <string.h>
#include <cassert>
#include <sstream>
#include <iostream>

TurnSystem::TurnSystem(const PathFindingSystem& pfs)
		: pathFindingSystem(pfs)
{
		EventSystem<MouseClickEvent>::instance().registerListener(
				std::bind(&TurnSystem::onMouseClick, this, std::placeholders::_1));

		EventSystem<PlayerButtonEvent>::instance().registerListener(
				std::bind(&TurnSystem::onPlayerButtonClick, this, std::placeholders::_1));

	EventSystem<FinishedMovementEvent>::instance().registerListener(
			std::bind(&TurnSystem::onFinishedMovement, this, std::placeholders::_1));

	EventSystem<FinishedSkillEvent>::instance().registerListener(
			std::bind(&TurnSystem::onFinishedSkill, this, std::placeholders::_1));
}

TurnSystem::~TurnSystem()
{
		if (mouseClickListener.isValid())
		{
			EventSystem<MouseClickEvent>::instance().unregisterListener(mouseClickListener);
		}
		if (playerButtonListener.isValid())
		{
			EventSystem<MouseClickEvent>::instance().unregisterListener(playerButtonListener);
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

				// Make sure the entity is alive
				if (!entity.has<DeathTimer>())
				{
						auto& turnComponent = ECS::registry<TurnComponent>.get(entity);
						if (!hasCompletedTurn(turnComponent)) {
								ECS::registry<TurnComponentIsActive>.emplace(entity);
								EventSystem<PlayerChangeEvent>::instance().sendEvent(PlayerChangeEvent{ entity });
								break;
						}
				}
		}

		//if all the players have gone start going through all the mobs
		if (ECS::registry<TurnComponentIsActive>.size() == 0) {
				auto& registry = ECS::registry<AISystem::MobComponent>;
				for (unsigned int i = 0; i < registry.components.size(); i++) {
						ECS::Entity entity = registry.entities[i];
						auto& turnComponent = ECS::registry<TurnComponent>.get(entity);
						if (!hasCompletedTurn(turnComponent)) {
								ECS::registry<TurnComponentIsActive>.emplace(entity);
								EventSystem<PlayerChangeEvent>::instance().sendEvent(PlayerChangeEvent{ entity });
								break;
						}
				}
		}

		if (ECS::registry<TurnComponentIsActive>.size() == 0) {
				//All entities have gone so end the turn
				nextTurn();
		}
}

//Sets the current entity to the specified "nextEntity"
void TurnSystem::changeActiveEntity(ECS::Entity nextEntity)
{
		//Make sure the nextEntity has a TurnComponent
		assert(nextEntity.has<TurnComponent>());
		auto& turnComponent = ECS::registry<TurnComponent>.get(nextEntity);
		if (!hasCompletedTurn(turnComponent)) {
				//Check if the selected player is the current active player
				assert(!ECS::registry<TurnComponentIsActive>.entities.empty());
				if (nextEntity.id == ECS::registry<TurnComponentIsActive>.entities[0].id) {
						std::cout << "The requested player is already the active player\n";
				}
				else {
						std::cout << "switching to the next active entity \n";
						//Remove the active entity
						ECS::registry<TurnComponentIsActive>.clear();

						//Set the activeEntity to the nextEntity
						ECS::registry<TurnComponentIsActive>.emplace(nextEntity);
						EventSystem<PlayerChangeEvent>::instance().sendEvent(PlayerChangeEvent{ nextEntity });
				}
		}
		else {
				std::cout << "player has already gone this turn\n";
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
				turnComponent.isMoving = false;
				turnComponent.hasMoved = false;
				turnComponent.hasMoved = false;
				turnComponent.hasUsedSkill = false;
		}

		//start the next turn
		nextActiveEntity();

}

void TurnSystem::step(float elapsed_ms)
{
		//If there is no active entity (this could be due to a restart) get the next active entity
		if (ECS::registry<TurnComponentIsActive>.entities.empty()) {
				nextActiveEntity();
		}
		// Check happens with above if statement.
		auto& activeEntity = ECS::registry<TurnComponentIsActive>.entities[0];
		if (!activeEntity.has<DeathTimer>()) {
				if (activeEntity.has<TurnComponent>()) {
						auto& turnComponent = activeEntity.get<TurnComponent>();

						if (hasCompletedTurn(turnComponent))
						{
								nextActiveEntity();
						}
						// For mobs, need to tell them when its time to move and to perform a skill
						else if (activeEntity.has<AISystem::MobComponent>())
						{
								if (turnComponent.canStartMoving() && turnComponent.canStartSkill())
								{
										turnComponent.isMoving = true;
										turnComponent.isUsingSkill = true;

										StartMobTurnEvent event;
										event.entity = activeEntity;
										EventSystem<StartMobTurnEvent>::instance().sendEvent(event);
								}
						}
				}
		}
		else {	
				//Pretty sure whis will be needed once we get multiple players but that depends 
				//		on how we handle death leaving for now as it doesn't hurt anything
				//The current user is dead so switch to the next
				nextActiveEntity();
		}
}

bool TurnSystem::hasCompletedTurn(TurnComponent tc) 
{
	return tc.hasUsedSkill;
}

void TurnSystem::onMouseClick(const MouseClickEvent &event)
{
	if (!ECS::registry<TurnComponentIsActive>.entities.empty())
	{
		// Get the active entity
		// Check already occurs with if statement above
		auto& activeEntity = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];

		// Check that it's a player and has a TurnComponent
		if (activeEntity.has<PlayerComponent>() && activeEntity.has<TurnComponent>())
		{
			auto& turnComponent = activeEntity.get<TurnComponent>();

			if (turnComponent.canStartMoving())
			{
				// Motion component is mandatory
				assert(activeEntity.has<Motion>());

				auto& motion = activeEntity.get<Motion>();
				motion.path = pathFindingSystem.getShortestPath(motion.position, event.mousePos);

				// Only set isMoving to true if a path was actually generated. If the user clicked too close to their character,
				// then no path would be generated, for example
				turnComponent.isMoving = !motion.path.empty();
			}
			else if (turnComponent.canStartSkill())
			{
				turnComponent.isUsingSkill = true;

				PerformActiveSkillEvent performActiveSkillEvent;
				performActiveSkillEvent.entity = activeEntity;
				performActiveSkillEvent.target = event.mousePos;
				EventSystem<PerformActiveSkillEvent>::instance().sendEvent(performActiveSkillEvent);
			}
		}
	}
}

// Handles switching active player when a player button is clicked
void TurnSystem::onPlayerButtonClick(const PlayerButtonEvent& event) 
{
	// Can't change players if the current active player is busy with movement or a skill
	if (!ECS::registry<TurnComponentIsActive>.entities.empty())
	{
		auto activeEntity = ECS::registry<TurnComponentIsActive>.entities.front();

		if (activeEntity.has<TurnComponent>())
		{
			auto& turnComponent = activeEntity.get<TurnComponent>();

			if (turnComponent.isMoving || turnComponent.isUsingSkill)
			{
				return;
			}
		}
	}

	// look for player entity that matches player in event
	for (auto entity : ECS::registry<PlayerComponent>.entities) 
	{
		auto& player = entity.get<PlayerComponent>().player;
		if (event.player == player) {
			TurnSystem::changeActiveEntity(entity);
			break;
		}
	}
}

void TurnSystem::onFinishedMovement(const FinishedMovementEvent& event)
{
	auto entity = event.entity;

	if (entity.has<TurnComponent>())
	{
		auto& turnComponent = entity.get<TurnComponent>();

		turnComponent.isMoving = false;
		turnComponent.hasMoved = true;
	}
}

void TurnSystem::onFinishedSkill(const FinishedSkillEvent& event)
{
	auto entity = event.entity;

	if (entity.has<TurnComponent>())
	{
		auto& turnComponent = entity.get<TurnComponent>();

		turnComponent.isUsingSkill = false;
		turnComponent.hasUsedSkill = true;
	}
}
