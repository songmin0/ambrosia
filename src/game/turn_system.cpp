#include "turn_system.hpp"

#include "ai/ai.hpp"
#include "game/camera.hpp"
#include "rendering/render_components.hpp"
#include "game/game_state_system.hpp"

#include <string.h>
#include <cassert>
#include <sstream>
#include <iostream>

TurnSystem::TurnSystem(PathFindingSystem& pfs)
	: pathFindingSystem(pfs)
	, timer(0.f)
{
	mouseClickListener = EventSystem<MouseClickEvent>::instance().registerListener(
			std::bind(&TurnSystem::onMouseClick, this, std::placeholders::_1));

	playerButtonListener = EventSystem<PlayerButtonEvent>::instance().registerListener(
			std::bind(&TurnSystem::onPlayerButtonClick, this, std::placeholders::_1));

	finishedMovementListener = EventSystem<FinishedMovementEvent>::instance().registerListener(
			std::bind(&TurnSystem::onFinishedMovement, this, std::placeholders::_1));

	finishedSkillListener = EventSystem<FinishedSkillEvent>::instance().registerListener(
			std::bind(&TurnSystem::onFinishedSkill, this, std::placeholders::_1));

	prepForNextMapListener = EventSystem<PrepForNextMapEvent>::instance().registerListener(
			std::bind(&TurnSystem::onPrepForNextMap, this, std::placeholders::_1));
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
	if (finishedMovementListener.isValid())
	{
		EventSystem<FinishedMovementEvent>::instance().unregisterListener(finishedMovementListener);
	}
	if (finishedSkillListener.isValid())
	{
		EventSystem<FinishedSkillEvent>::instance().unregisterListener(finishedSkillListener);
	}
	if (prepForNextMapListener.isValid())
	{
		EventSystem<PrepForNextMapEvent>::instance().unregisterListener(prepForNextMapListener);
	}
}

//Just sets the next available entity as the current entity
void TurnSystem::nextActiveEntity()
{
	std::cout << "nextActiveEntity: switching to the next active entity \n";
	//Remove the active entity
	ECS::registry<TurnComponentIsActive>.clear();

	auto tryToSetActiveEntity = [](const vector<ECS::Entity> entities)
	{
		for (auto entity : entities)
		{
			auto& turnComponent = ECS::registry<TurnComponent>.get(entity);

			// Skip the turn of stunned entities
			if (entity.has<StatsComponent>() &&
					entity.get<StatsComponent>().isStunned())
			{
				turnComponent.hasUsedSkill = true;
				continue;
			}

			if (!hasCompletedTurn(turnComponent) && !entity.has<DeathTimer>())
			{
				ECS::registry<TurnComponentIsActive>.emplace(entity);
				EventSystem<PlayerChangeEvent>::instance().sendEvent(PlayerChangeEvent{ entity });
				break;
			}
		}
	};

	//Sort the vector of players that are alive based on the player enum. I had to create this temp variable because if i sorted directly on the registy it broke.
	auto playerEntities = ECS::registry<PlayerComponent>.entities;
	std::sort(playerEntities.begin(), playerEntities.end(), [](ECS::Entity a, ECS::Entity b) {
		return a.get<PlayerComponent>().player < b.get<PlayerComponent>().player;
		});
	// Loop through the player's TurnComponents and set the first player found that hasn't gone yet
	tryToSetActiveEntity(playerEntities);
	

	// If all the players have gone start going through all the mobs
	if (ECS::registry<TurnComponentIsActive>.size() == 0)
	{
		tryToSetActiveEntity(ECS::registry<AISystem::MobComponent>.entities);
	}

	if (ECS::registry<TurnComponentIsActive>.size() == 0)
	{
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
			std::cout << "\nSwitching to the next active entity\n";
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
	std::cout << "Starting the next turn\n";
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

	EventSystem<StartNextRoundEvent>::instance().sendEvent({});

	//start the next turn
	nextActiveEntity();

}

void TurnSystem::step(float elapsed_ms)
{
	if (!GameStateSystem::instance().inGameState()) {
		return;
	}
	timer -= elapsed_ms;

	// The turn system should only run if enough time has passed since the last
	// movement or skill was performed AND if there are no active projectiles
	// (e.g., ambrosia projectiles)
	if (timer > 0.f || !ECS::registry<ProjectileComponent>.components.empty())
	{
		return;
	}
	timer = 0.f;

	//If there is no active entity (this could be due to a restart) get the next active entity
	if (ECS::registry<TurnComponentIsActive>.entities.empty()) {
		std::cout << "There is no active entity\n";
		nextActiveEntity();
	}
	// Check happens with above if statement.
	auto& activeEntity = ECS::registry<TurnComponentIsActive>.entities[0];
	if (!activeEntity.has<DeathTimer>()) 
	{
		if (activeEntity.has<TurnComponent>()) 
		{
			auto& turnComponent = activeEntity.get<TurnComponent>();
			
			if (hasCompletedTurn(turnComponent))
			{
				if (activeEntity.has<AISystem::MobComponent>())
				{
					std::cout << "Mob turn ended\n";
					EventSystem<EndMobTurnEvent>::instance().sendEvent(EndMobTurnEvent{});
				}
				// Add a hardcoded delay before moving the camera so player can see animations
				assert(!ECS::registry<CameraComponent>.entities.empty());
				auto camera = ECS::registry<CameraComponent>.entities[0];
				camera.emplace<CameraDelayedMoveComponent>(0.4f);
				nextActiveEntity();
			}
			// For mobs, need to tell them when its time to move and to perform a skill
			else if (activeEntity.has<AISystem::MobComponent>())
			{
				// Start behaviour tree for active mob entity if there are players left
				if (turnComponent.canStartMoving() && this->playersLeft() &&
						activeEntity.has<StatsComponent>() &&
						!activeEntity.get<StatsComponent>().isStunned())
				{
					std::cout << "Starting mob turn\n";
					// Event should be heard by StateSystem in behaviour_tree.hpp
					EventSystem<StartMobTurnEvent>::instance().sendEvent(StartMobTurnEvent{});
					turnComponent.isMoving = true;
				}
			}
		}
	}
	else
	{
		//The current user is dead so switch to the next
		std::cout << "current user is dead\n";
		nextActiveEntity();
		
	}
}

// Helper function to check that there are players to fight against before starting mob turn
bool TurnSystem::playersLeft()
{
	bool playersLeft = false;
	auto& playerContainer = ECS::registry<PlayerComponent>;
	for (ECS::Entity player : playerContainer.entities)
	{
		if (!player.has<DeathTimer>())
			playersLeft = true;
	}
	return playersLeft;
}

bool TurnSystem::hasCompletedTurn(TurnComponent tc)
{
	return tc.hasUsedSkill;
}

void TurnSystem::onMouseClick(const MouseClickEvent& event)
{
	if (ECS::registry<TurnComponentIsActive>.entities.empty())
	{
		return;
	}

	auto& activeEntity = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];

	// Check that it's a player and has a TurnComponent
	if (activeEntity.has<PlayerComponent>() && activeEntity.has<TurnComponent>())
	{
		auto& turnComponent = activeEntity.get<TurnComponent>();

		if (activeEntity.has<StatsComponent>() &&
		    activeEntity.get<StatsComponent>().isStunned())
		{
			return;
		}

		if (turnComponent.activeAction == SkillType::MOVE && turnComponent.canStartMoving())
		{
			// Motion component is mandatory
			assert(activeEntity.has<Motion>());

			auto& motion = activeEntity.get<Motion>();
			motion.path = pathFindingSystem.getShortestPath(activeEntity, event.mousePos);

			// Only set isMoving to true if a path was actually generated. If the user clicked too close to their character,
			// then no path would be generated, for example
			turnComponent.isMoving = !motion.path.empty();
		}
		else if (turnComponent.canStartSkill())
		{
			// Don't use a skill if in Null State
			if (activeEntity.get<SkillComponent>().getActiveSkillType() == SkillType::NONE)
			{
				return;
			}

			turnComponent.isUsingSkill = true;

			PerformActiveSkillEvent performActiveSkillEvent;
			performActiveSkillEvent.entity = activeEntity;
			performActiveSkillEvent.target = event.mousePos;
			EventSystem<PerformActiveSkillEvent>::instance().sendEvent(performActiveSkillEvent);
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

		// Reset to Null state
		entity.get<SkillComponent>().setActiveSkill(SkillType::NONE);
		turnComponent.activeAction = SkillType::NONE;

		if (GameStateSystem::instance().isInTutorial && GameStateSystem::instance().currentTutorialIndex == 4)
		{
			EventSystem<AdvanceTutorialEvent>::instance().sendEvent(AdvanceTutorialEvent{});
		}
	}

	timer = TIMER_PERIOD;
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

	timer = TIMER_PERIOD;
}

void TurnSystem::onPrepForNextMap(const PrepForNextMapEvent& event)
{
	auto entity = event.entity;
	if (entity.has<TurnComponent>())
	{
		auto& turnComponent = entity.get<TurnComponent>();
		turnComponent.isMoving = false;
		turnComponent.isUsingSkill = false;
		turnComponent.hasMoved = false;
		turnComponent.hasUsedSkill = false;
		turnComponent.activeAction = SkillType::NONE;
	}

	entity.remove<TurnComponentIsActive>();
}
