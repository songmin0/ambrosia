#include "ai.hpp"
#include "swarm_behaviour.hpp"

#include "entities/tiny_ecs.hpp"
#include "physics/physics.hpp"
#include "game/turn_system.hpp"
#include "game/game_state_system.hpp"

#include <iostream>

AISystem::AISystem(PathFindingSystem& pfs)
	: pathFindingSystem(pfs)
{
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());

	startMobMoveListener = EventSystem<StartMobMoveEvent>::instance().registerListener(
		std::bind(&AISystem::onStartMobMoveEvent, this, std::placeholders::_1));

	startMobSkillListener = EventSystem<StartMobSkillEvent>::instance().registerListener(
			std::bind(&AISystem::onStartMobSkillEvent, this, std::placeholders::_1));
}

AISystem::~AISystem()
{
	if (startMobMoveListener.isValid())
	{
		EventSystem<StartMobMoveEvent>::instance().unregisterListener(startMobMoveListener);
	}

	if (startMobSkillListener.isValid())
	{
		EventSystem<StartMobSkillEvent>::instance().unregisterListener(startMobSkillListener);
	}
}

ECS::Entity AISystem::MobComponent::getTarget()
{
	return this->target;
}

void AISystem::MobComponent::setTarget(ECS::Entity target)
{
	this->target = target;
}

bool AISystem::setTargetToClosestPlayer(ECS::Entity & mob)
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
	mobComponent.setTarget(closestPlayer);
	return closestDistance != float_max;
}

bool AISystem::setTargetToFarthestPlayer(ECS::Entity& mob)
{
	// Movement for mobs - move to farthest player
	ECS::Entity farthestPlayer;
	// Given mob variable should be a mob
	assert(mob.has<MobComponent>());
	auto& mobComponent = mob.get<MobComponent>();
	auto& mobMotion = mob.get<Motion>();

	auto& playerContainer = ECS::registry<PlayerComponent>;
	// There should always be at least one player in a game
	assert(!playerContainer.entities.empty());

	// Find the farthest living player
	constexpr auto float_min = std::numeric_limits<float>::min();
	float farthestDistance = float_min;

	for (auto player : playerContainer.entities)
	{
		// Check that player is alive and has Motion component
		if (!player.has<DeathTimer>() && player.has<Motion>())
		{
			// Calculate the distance to this player
			auto& playerMotion = player.get<Motion>();
			float playerDistance = distance(mobMotion.position, playerMotion.position);

			// If this player is farther, update the farthest player
			if (playerDistance > farthestDistance)
			{
				farthestDistance = playerDistance;
				farthestPlayer = player;
			}
		}
	}
	mobComponent.setTarget(farthestPlayer);
	return farthestDistance != float_min;
}

bool AISystem::setTargetToWeakestPlayer(ECS::Entity& mob)
{
	// Movement for mobs - move to weakest player (lowest HP)
	ECS::Entity weakestPlayer;
	// Given mob variable should be a mob
	assert(mob.has<MobComponent>());
	auto& mobComponent = mob.get<MobComponent>();

	auto& playerContainer = ECS::registry<PlayerComponent>;
	// There should always be at least one player in a game
	assert(!playerContainer.entities.empty());

	// Find the player with lowest HP
	constexpr auto float_max = std::numeric_limits<float>::max();
	float lowestHP = float_max;
	for (auto& player : playerContainer.entities)
	{
		// Check that player is alive and has Motion component
		if (!player.has<DeathTimer>() && player.has<Motion>())
		{
			// Check HP
			auto& playerStats = player.get<StatsComponent>();
			auto playerHP = playerStats.getStatValue(StatType::HP);

			if (playerHP < playerStats.getStatValue(StatType::MAX_HP) && playerHP < lowestHP)
			{
				lowestHP = playerHP;
				weakestPlayer = player;
			}
		}
	}
	mobComponent.setTarget(weakestPlayer);
	return lowestHP != float_max;
}

bool AISystem::setTargetToRandomPlayer(ECS::Entity& mob)
{
	// Movement for mobs - move to a random player
	assert(mob.has<MobComponent>());
	auto& mobComponent = mob.get<MobComponent>();

	auto& playerContainer = ECS::registry<PlayerComponent>;
	assert(!playerContainer.entities.empty());

	std::vector<ECS::Entity> playerEntities(playerContainer.entities);
	std::shuffle(playerEntities.begin(), playerEntities.end(), rng);

	for (auto player : playerEntities)
	{
		if (!player.has<DeathTimer>() && player.has<Motion>())
		{
			mobComponent.setTarget(player);
			return true;
		}
	}

	return false;
}

bool AISystem::setTargetToWeakestMob(ECS::Entity& mob)
{
	ECS::Entity targetAlly;
	// Given mob variable should be a mob
	assert(mob.has<MobComponent>());
	auto& mobComponent = mob.get<MobComponent>();

	auto& mobContainer = ECS::registry<MobComponent>;
	// When called, there should always be current active mob and one ally
	assert(mobContainer.entities.size() > 1);

	// Find the ally with lowest HP
	constexpr auto float_max = std::numeric_limits<float>::max();
	float lowestHP = float_max;

	for (auto& ally : mobContainer.entities)
	{
		// Check for all other mobs that are alive and has Motion component
		if (!(ally.id == mob.id) && !ally.has<DeathTimer>() && ally.has<Motion>())
		{
			auto& allyStats = ally.get<StatsComponent>();
			auto allyHP = allyStats.getStatValue(StatType::HP);
			// Update ally to find mob with lowest HP
			if (allyHP < allyStats.getStatValue(StatType::MAX_HP) && allyHP < lowestHP)
			{
				lowestHP = allyHP;
				targetAlly = ally;
			}
		}
	}
	mobComponent.setTarget(targetAlly);
	return lowestHP != float_max;
}

bool AISystem::setTargetToDeadPotato(ECS::Entity& mob)
{
	ECS::Entity targetAlly;
	// Given mob variable should be a mob
	assert(mob.has<MobComponent>());
	auto& mobComponent = mob.get<MobComponent>();

	//// set target as dead potato
	mobComponent.setTarget(ECS::registry<ActivePotatoChunks>.get(mob).potato);
	ECS::Entity target = mobComponent.getTarget();

	return true;
}

void AISystem::startMobMove(ECS::Entity entity, MovementType movement)
{
	assert(entity.has<MobComponent>());
	assert(entity.has<Motion>());
	auto& motion = entity.get<Motion>();

	ECS::Entity target = entity.get<MobComponent>().getTarget();
	//Find the direction to travel based on movement type
	vec2 direction = { 0.f, 0.f };
	float magnitude = 0.f;
	if (movement.moveType == MoveType::AWAY_CLOSEST_PLAYER)
	{
		direction = normalize(motion.position - target.get<Motion>().position);
		magnitude = motion.moveRange;
	}
	else
	{
		direction = normalize(target.get<Motion>().position - motion.position);
		magnitude = length(target.get<Motion>().position - motion.position);
		// Limit desired distance by allowed movement range
		if (magnitude > motion.moveRange)
			magnitude = motion.moveRange;
	}

	vec2 destination = motion.position + (direction * magnitude);
	motion.path = pathFindingSystem.getShortestPath(entity, destination);
}

void AISystem::startMobSkill(ECS::Entity entity)
{
	assert(entity.has<MobComponent>());
	auto& mobComponent = entity.get<MobComponent>();

	ECS::Entity target = mobComponent.getTarget();
	// Get position of target
	assert(target.has<Motion>());
	vec2 targetPosition = target.get<Motion>().position;

	PerformActiveSkillEvent performActiveSkillEvent;
	performActiveSkillEvent.entity = entity;
	performActiveSkillEvent.target = targetPosition;
	EventSystem<PerformActiveSkillEvent>::instance().sendEvent(performActiveSkillEvent);
}

void AISystem::onStartMobMoveEvent(const StartMobMoveEvent& event)
{
	ECS::Entity entity = event.entity;
	bool targetExists = false;
	// Set the correct target depending on target type
	// This is important within startMobMove which calls getTarget()
	switch (event.movement.moveType)
	{
	case MoveType::TO_CLOSEST_PLAYER:
	case MoveType::AWAY_CLOSEST_PLAYER:
		targetExists = setTargetToClosestPlayer(entity);
		break;
	case MoveType::TO_FARTHEST_PLAYER:
		targetExists = setTargetToFarthestPlayer(entity);
		break;
	case MoveType::TO_WEAKEST_PLAYER:
		targetExists = setTargetToWeakestPlayer(entity);
		break;
	case MoveType::TO_WEAKEST_MOB:
		targetExists = setTargetToWeakestMob(entity);
		break;
	case MoveType::TO_DEAD_POTATO:
		targetExists = setTargetToDeadPotato(entity);
		break;
	case MoveType::TO_RANDOM_PLAYER:
		targetExists = setTargetToRandomPlayer(entity);
		break;
	default:
		break;
	}
	// Mob turn should only start when players are alive
	assert(targetExists);
	startMobMove(entity, event.movement);
}

void AISystem::onStartMobSkillEvent(const StartMobSkillEvent& event)
{
	ECS::Entity entity = event.entity;
	bool targetExists = false;
	// Set the correct target depending on target type
	// This is important within startMobSkill which calls getTarget()
	// Overrides others if the target is random
	if (event.isRandomTarget)
	{
		targetExists = setTargetToRandomPlayer(entity);
	}
	else if (event.targetIsPlayer)
	{
		targetExists = setTargetToClosestPlayer(entity);
	}
	else
	{
		targetExists = setTargetToWeakestMob(entity);
	}

	// Mob turn should only start when players are alive
	assert(targetExists);
	startMobSkill(event.entity);
}
