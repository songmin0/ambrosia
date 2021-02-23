#include "entity_filter.hpp"

std::vector<ECS::Entity> InstigatorFilter::process(const SkillParams& params, const std::vector<ECS::Entity>& entities)
{
	std::vector<ECS::Entity> results;

	for (auto entity : entities)
	{
		if (entity.id != params.instigator.id)
		{
			results.push_back(entity);
		}
	}

	return results;
}

std::vector<ECS::Entity> CollisionFilter::process(const SkillParams& params, const std::vector<ECS::Entity> &entities)
{
	std::vector<ECS::Entity> results;

	for (auto entity : entities)
	{
		if (entity.get<Motion>().colliderType & params.collidesWith)
		{
			results.push_back(entity);
		}
	}

	return results;
}
