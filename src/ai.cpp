// internal
#include "ai.hpp"
#include "tiny_ecs.hpp"
#include "physics.hpp"
#include "TurnSystem.hpp"

#include <iostream>

AISystem::AISystem(const PathFindingSystem& pfs)
	: pathFindingSystem(pfs)
{
}

void AISystem::step(float elapsed_ms, vec2 window_size_in_game_units)
{
	for (ECS::Entity entity : ECS::registry<MobComponent>.entities)
	{
		//if (entity.has<TurnSystem::TurnComponentIsActive>())
		if (entity.get<Motion>().path.empty())
		{
			auto& motion = entity.get<Motion>();
			// Movement for mobs - move to closest player
			auto& playerContainer = ECS::registry<PlayerComponent>;
			ECS::Entity closestPlayer = playerContainer.entities[0]; // Initialize to first player
			float closestPlayerDistance = distance(motion.position, closestPlayer.get<Motion>().position);
			// If there is more than one player
			for (unsigned int j = 1; j < playerContainer.components.size(); j++)
			{
				ECS::Entity player = playerContainer.entities[j];
				auto& playerMotion = player.get<Motion>();
				if (distance(motion.position, playerMotion.position) < closestPlayerDistance)
				{
					closestPlayer = player;
				}
			}
			entity.get<AISystem::MobComponent>().SetTargetEntity(closestPlayer);
			motion.path = pathFindingSystem.GetShortestPath(motion.position, closestPlayer.get<Motion>().position);
		}
	}
	
	(void)elapsed_ms; // placeholder to silence unused warning until implemented
	(void)window_size_in_game_units; // placeholder to silence unused warning until implemented
}

ECS::Entity AISystem::MobComponent::GetTargetEntity()
{
	return this->target;
}

void AISystem::MobComponent::SetTargetEntity(ECS::Entity target)
{
	this->target = target;
}
