#include "ai.hpp"

#include "entities/tiny_ecs.hpp"
#include "physics/physics.hpp"
#include "game/turn_system.hpp"

#include <iostream>

AISystem::AISystem(const PathFindingSystem& pfs)
	: pathFindingSystem(pfs)
{
	startMobMovementListener = EventSystem<StartMobMovementEvent>::instance().registerListener(
			std::bind(&AISystem::onStartMobMovementEvent, this, std::placeholders::_1));

	startMobSkillListener = EventSystem<StartMobSkillEvent>::instance().registerListener(
			std::bind(&AISystem::onStartMobSkillEvent, this, std::placeholders::_1));
}

AISystem::~AISystem()
{
	if (startMobMovementListener.isValid())
	{
		EventSystem<StartMobMovementEvent>::instance().unregisterListener(startMobMovementListener);
	}

	if (startMobSkillListener.isValid())
	{
		EventSystem<StartMobSkillEvent>::instance().unregisterListener(startMobSkillListener);
	}
}

void AISystem::step(float elapsed_ms, vec2 window_size_in_game_units)
{
	(void)elapsed_ms; // placeholder to silence unused warning until implemented
	(void)window_size_in_game_units; // placeholder to silence unused warning until implemented
}

ECS::Entity AISystem::MobComponent::getTargetEntity()
{
	return this->target;
}

void AISystem::MobComponent::setTargetEntity(ECS::Entity target)
{
	this->target = target;
}

bool AISystem::getClosestPlayer(ECS::Entity& mob)
{
	// Movement for mobs - move to closest player
	ECS::Entity closestPlayer;
	// Given mob variable should be a mob
	assert(mob.has<MobComponent>());
	auto& mobComponent = mob.get<MobComponent>();
	auto& mobMotion = mob.get<Motion>();

	auto& playerContainer = ECS::registry<PlayerComponent>;
	// There should always be at least one player in a game
	assert(!playerContainer.entities.empty());

	// Find the closest living player
	constexpr auto float_max = std::numeric_limits<float>::max();
	float closestDistance = float_max;

	for (auto player : playerContainer.entities)
	{
		// Check that player is alive and has Motion component
		if (!player.has<DeathTimer>() && player.has<Motion>())
		{
			// Calculate the distance to this player
			auto& playerMotion = player.get<Motion>();
			float playerDistance = distance(mobMotion.position, playerMotion.position);

			// If this player is closer, update the closest player
			if (playerDistance < closestDistance)
			{
				closestDistance = playerDistance;
				closestPlayer = player;
			}
		}
	}
	mobComponent.setTargetEntity(closestPlayer);
	return closestDistance != float_max;
}

void AISystem::startMobMovement(ECS::Entity entity)
{
	assert(entity.has<MobComponent>());
	// Motion component is mandatory
	assert(entity.has<Motion>());
	auto& motion = entity.get<Motion>();

	std::cout << "finding path\n";

	if (getClosestPlayer(entity)) {
		ECS::Entity closestPlayer = entity.get<MobComponent>().target;
		//Find the direction to travel towards the player
		vec2 direction = normalize(closestPlayer.get<Motion>().position - motion.position);

		//Calculate the point to walk to
		//TODO properly define and decide how far a mob can move in a turn
		float movementDistance = 100.0f;
		vec2 destintation = motion.position + (direction * movementDistance);
		motion.path = pathFindingSystem.getShortestPath(motion.position, destintation);
	}
}

void AISystem::startMobSkill(ECS::Entity entity)
{
	// Motion component is mandatory
	assert(entity.has<Motion>());
	auto& motion = entity.get<Motion>();
	assert(entity.has<MobComponent>());
	auto& mobComponent = entity.get<MobComponent>();

	ECS::Entity closestPlayer = mobComponent.target;
	// Get position of closest player
	assert(closestPlayer.has<Motion>());
	vec2 closestPlayerPosition = closestPlayer.get<Motion>().position;

	PerformActiveSkillEvent performActiveSkillEvent;
	performActiveSkillEvent.entity = entity;
	performActiveSkillEvent.target = closestPlayerPosition;
	EventSystem<PerformActiveSkillEvent>::instance().sendEvent(performActiveSkillEvent);
}

void AISystem::onStartMobMovementEvent(const StartMobMovementEvent& event)
{
	startMobMovement(event.entity);
}

void AISystem::onStartMobSkillEvent(const StartMobSkillEvent& event)
{
	startMobSkill(event.entity);
}
