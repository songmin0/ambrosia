#include "ai.hpp"

#include "entities/tiny_ecs.hpp"
#include "physics/physics.hpp"
#include "game/turn_system.hpp"

#include <iostream>

AISystem::AISystem(PathFindingSystem& pfs)
	: pathFindingSystem(pfs)
{
	startMobMoveToPlayerListener = EventSystem<StartMobMoveToPlayerEvent>::instance().registerListener(
			std::bind(&AISystem::onStartMobMoveToPlayerEvent, this, std::placeholders::_1));

	startMobMoveToMobListener = EventSystem<StartMobMoveToMobEvent>::instance().registerListener(
		std::bind(&AISystem::onStartMobMoveToMobEvent, this, std::placeholders::_1));

	startMobRunAwayListener = EventSystem<StartMobRunAwayEvent>::instance().registerListener(
		std::bind(&AISystem::onStartMobRunAwayEvent, this, std::placeholders::_1));

	startMobSkillListener = EventSystem<StartMobSkillEvent>::instance().registerListener(
			std::bind(&AISystem::onStartMobSkillEvent, this, std::placeholders::_1));
}

AISystem::~AISystem()
{
	if (startMobMoveToPlayerListener.isValid())
	{
		EventSystem<StartMobMoveToPlayerEvent>::instance().unregisterListener(startMobMoveToPlayerListener);
	}

	if (startMobMoveToMobListener.isValid())
	{
		EventSystem<StartMobMoveToMobEvent>::instance().unregisterListener(startMobMoveToMobListener);
	}

	if (startMobRunAwayListener.isValid())
	{
		EventSystem<StartMobRunAwayEvent>::instance().unregisterListener(startMobRunAwayListener);
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

ECS::Entity AISystem::MobComponent::getTarget()
{
	return this->target;
}

void AISystem::MobComponent::setTarget(ECS::Entity target)
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
	mobComponent.setTarget(closestPlayer);
	return closestDistance != float_max;
}

bool AISystem::getClosestMob(ECS::Entity& mob)
{
	// Movement for mobs - move to closest mob
	// Just milk for now
	ECS::Entity closestAlly;
	// Given mob variable should be a mob
	assert(mob.has<MobComponent>());
	auto& mobComponent = mob.get<MobComponent>();
	auto& mobMotion = mob.get<Motion>();

	auto& mobContainer = ECS::registry<MobComponent>;
	// When called, there should always be current active mob and one ally
	assert(mobContainer.entities.size() > 1);

	// Find the closest living mob
	constexpr auto float_max = std::numeric_limits<float>::max();
	float closestDistance = float_max;

	for (auto ally : mobContainer.entities)
	{
		// Check for all other mobs that is alive and has Motion component
		if (!(ally.id == mob.id) && !ally.has<DeathTimer>() && ally.has<Motion>())
		{
			// Calculate the distance to this mob
			auto& allyMotion = ally.get<Motion>();
			float allyDistance = distance(mobMotion.position, allyMotion.position);

			// If this player is closer, update the closest player
			if (allyDistance < closestDistance)
			{
				closestDistance = allyDistance;
				closestAlly = ally;
			}
		}
	}
	mobComponent.setTarget(closestAlly);
	return closestDistance != float_max;
}

void AISystem::startMobMoveToPlayer(ECS::Entity entity)
{
	assert(entity.has<MobComponent>());
	// Motion component is mandatory
	assert(entity.has<Motion>());
	auto& motion = entity.get<Motion>();

	std::cout << "Moving towards player\n";

	if (getClosestPlayer(entity)) {
		ECS::Entity closestPlayer = entity.get<MobComponent>().getTarget();
		//Find the direction to travel towards the player
		vec2 direction = normalize(closestPlayer.get<Motion>().position - motion.position);

		//Calculate the point to walk to
		//TODO properly define and decide how far a mob can move in a turn
		float movementDistance = 100.0f;
		vec2 destintation = motion.position + (direction * movementDistance);
		motion.path = pathFindingSystem.getShortestPath(entity, destintation);
	}
}

void AISystem::startMobMoveToMob(ECS::Entity entity)
{
	assert(entity.has<MobComponent>());
	// Motion component is mandatory
	assert(entity.has<Motion>());
	auto& motion = entity.get<Motion>();

	std::cout << "Moving towards mob ally\n";

	if (getClosestMob(entity)) {
		ECS::Entity closestMob = entity.get<MobComponent>().getTarget();
		//Find the direction to travel towards the player
		vec2 direction = normalize(closestMob.get<Motion>().position - motion.position);

		//Calculate the point to walk to
		//TODO properly define and decide how far a mob can move in a turn
		float movementDistance = 100.0f;
		vec2 destintation = motion.position + (direction * movementDistance);
		motion.path = pathFindingSystem.getShortestPath(entity, destintation);
	}
}

void AISystem::startMobRunAway(ECS::Entity entity)
{
	assert(entity.has<MobComponent>());
	assert(entity.has<Motion>());
	auto& motion = entity.get<Motion>();

	std::cout << "Running away from player\n";

	if (getClosestPlayer(entity)) {
		ECS::Entity closestPlayer = entity.get<MobComponent>().getTarget();
		//Find the direction to travel away from the player
		vec2 direction = normalize(motion.position - closestPlayer.get<Motion>().position);

		float movementDistance = 100.0f;
		vec2 destintation = motion.position + (direction * movementDistance);
		motion.path = pathFindingSystem.getShortestPath(entity, destintation);
	}
}

void AISystem::startMobSkill(ECS::Entity entity)
{
	// Motion component is mandatory
	assert(entity.has<Motion>());
	assert(entity.has<MobComponent>());
	auto& motion = entity.get<Motion>();
	auto& mobComponent = entity.get<MobComponent>();

	std::cout << "Mob using skill\n";

	// Skill currently targets the closest player
	// TODO: This can change with cooperative actions like buffing between mobs
	ECS::Entity target = mobComponent.getTarget();
	// Get position of closest player
	assert(target.has<Motion>());
	vec2 closestTargetPosition = target.get<Motion>().position;

	PerformActiveSkillEvent performActiveSkillEvent;
	performActiveSkillEvent.entity = entity;
	performActiveSkillEvent.target = closestTargetPosition;
	EventSystem<PerformActiveSkillEvent>::instance().sendEvent(performActiveSkillEvent);
}

void AISystem::onStartMobMoveToPlayerEvent(const StartMobMoveToPlayerEvent& event)
{
	startMobMoveToPlayer(event.entity);
}

void AISystem::onStartMobMoveToMobEvent(const StartMobMoveToMobEvent& event)
{
	startMobMoveToMob(event.entity);
}

void AISystem::onStartMobRunAwayEvent(const StartMobRunAwayEvent& event)
{
	startMobRunAway(event.entity);
}

void AISystem::onStartMobSkillEvent(const StartMobSkillEvent& event)
{
	startMobSkill(event.entity);
}
