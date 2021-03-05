#include "entity_provider.hpp"

std::vector<ECS::Entity> EntityProvider::getSortedEntities(std::vector<std::pair<ECS::Entity, float>>& entities)
{
	// Sort so that the closest entities are at the front of the list
	std::sort(entities.begin(), entities.end(), [](const auto& pair1, const auto& pair2) {
		return pair1.second < pair2.second;
	});

	std::vector<ECS::Entity> sortedEntities;
	sortedEntities.reserve(entities.size());

	// Build the list of sorted entities
	for (const auto& pair : entities)
	{
		sortedEntities.push_back(pair.first);
	}

	return sortedEntities;
}

std::vector<ECS::Entity> CircularProvider::getEntities(const SkillParams& params)
{
	// Pairs of entities and their distances to the instigator
	std::vector<std::pair<ECS::Entity, float>> entities;

	for (int i = 0; i < ECS::registry<Motion>.entities.size(); i++)
	{
		auto entity = ECS::registry<Motion>.entities[i];
		auto& motion = ECS::registry<Motion>.components[i];

		float dist = distance(params.sourcePosition, motion.position);
		if (dist <= params.range)
		{
			entities.emplace_back(entity, dist);
		}
	}

	return getSortedEntities(entities);
}

std::vector<ECS::Entity> ConicalProvider::getEntities(const SkillParams &params)
{
	// Pairs of entities and their angular distance
	std::vector<std::pair<ECS::Entity, float>> entities;

	vec2 sourceToTarget = normalize(params.targetPosition - params.sourcePosition);

	for (int i = 0; i < ECS::registry<Motion>.entities.size(); i++)
	{
		auto entity = ECS::registry<Motion>.entities[i];
		auto& motion = ECS::registry<Motion>.components[i];

		vec2 sourceToEntity = normalize(motion.position - params.sourcePosition);

		float entityAngle = acos(dot(sourceToTarget, sourceToEntity));
		if (entityAngle <= params.angle)
		{
			entities.emplace_back(entity, entityAngle);
		}
	}

	return getSortedEntities(entities);
}

std::vector<ECS::Entity> MouseClickProvider::getEntities(const SkillParams &params)
{
	// Pairs of entities and their distance to the mouse click
	std::vector<std::pair<ECS::Entity, float>> entities;

	for (int i = 0; i < ECS::registry<Motion>.entities.size(); i++)
	{
		auto entity = ECS::registry<Motion>.entities[i];
		auto& motion = ECS::registry<Motion>.components[i];

		float dist = distance(params.targetPosition, motion.position);
		if (dist <= params.range)
		{
			entities.emplace_back(entity, dist);
		}
	}

	return getSortedEntities(entities);
}
