#include "state_system.hpp"

StateSystem::StateSystem()
{
	for (ECS::Entity entity : ECS::registry<AISystem::MobComponent>.entities)
	{
		auto& mobComponent = entity.get<AISystem::MobComponent>();
		if (mobComponent.mobType == MobType::MOB)
		{
		}
	}
}