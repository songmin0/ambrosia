#include "entity_provider.hpp"

std::vector<ECS::Entity> CircularProvider::getEntities(const SkillParams& params)
{
	std::vector<ECS::Entity> entities;

	for (int i = 0; i < ECS::registry<Motion>.entities.size(); i++)
	{
		auto entity = ECS::registry<Motion>.entities[i];
		auto& motion = ECS::registry<Motion>.components[i];

		if (distance(params.sourcePosition, motion.position) <= params.range)
		{
			entities.push_back(entity);
		}
	}

	return entities;
}

std::vector<ECS::Entity> ConicalProvider::getEntities(const SkillParams &params)
{
	std::vector<ECS::Entity> entities;

	vec2 sourceToTarget = normalize(params.targetPosition - params.sourcePosition);

	for (int i = 0; i < ECS::registry<Motion>.entities.size(); i++)
	{
		auto entity = ECS::registry<Motion>.entities[i];
		auto& motion = ECS::registry<Motion>.components[i];

		vec2 sourceToEntity = normalize(motion.position - params.sourcePosition);

		if (acos(dot(sourceToTarget, sourceToEntity)) <= params.angle)
		{
			entities.push_back(entity);
		}
	}

	return entities;
}

std::vector<ECS::Entity> MouseClickProvider::getEntities(const SkillParams &params)
{
	std::vector<ECS::Entity> entities;

	for (int i = 0; i < ECS::registry<Motion>.entities.size(); i++)
	{
		auto entity = ECS::registry<Motion>.entities[i];
		auto& motion = ECS::registry<Motion>.components[i];

		if (distance(params.targetPosition, motion.position) <= params.range)
		{
			entities.push_back(entity);
		}
	}

	return entities;
}
