#include "entity_provider.hpp"

namespace
{
	vec2 getCenterOfEntity(ECS::Entity entity)
	{
		assert(entity.has<Motion>());
		auto& motion = entity.get<Motion>();

		vec2 center = motion.position;
		center.y -= abs(motion.boundingBox.y / 2.f);

		return center;
	}

	vec2 getClosestPointOnBoundingBox(vec2 source, const Motion& targetMotion)
	{
		vec2 boundingBox = abs(targetMotion.boundingBox);
		vec2 closestPoint = source;

		float boundingBoxHalfWidth = boundingBox.x / 2.f;
		if (source.x < (targetMotion.position.x - boundingBoxHalfWidth))
		{
			// The closest point is somewhere on the left edge of the bounding box
			closestPoint.x = targetMotion.position.x - boundingBoxHalfWidth;
		}
		else if (source.x > (targetMotion.position.x + boundingBoxHalfWidth))
		{
			// The closest point is somewhere on the right edge of the bounding box
			closestPoint.x = targetMotion.position.x + boundingBoxHalfWidth;
		}

		if (source.y < (targetMotion.position.y - boundingBox.y))
		{
			// The closest point is somewhere on the top edge of the bounding box
			closestPoint.y = targetMotion.position.y - boundingBox.y;
		}
		else if (source.y > targetMotion.position.y)
		{
			// The closest point is somewhere on the bottom edge of the bounding box
			closestPoint.x = targetMotion.position.y;
		}

		return closestPoint;
	}

	std::vector<ECS::Entity> getSortedEntities(std::vector<std::pair<ECS::Entity, float>>& entities)
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
}

std::vector<ECS::Entity> CircularProvider::getEntities(ECS::Entity instigator,
																											 vec2 targetPosition)
{
	// Pairs of entities and their distances to the instigator
	std::vector<std::pair<ECS::Entity, float>> entities;

	assert(instigator.has<Motion>());
	vec2 instigatorPosition = instigator.get<Motion>().position;

	// We should search in a circle around the center of the instigator
	vec2 centerOfInstigator = getCenterOfEntity(instigator);

	for (int i = 0; i < ECS::registry<Motion>.entities.size(); i++)
	{
		auto entity = ECS::registry<Motion>.entities[i];
		auto& motion = ECS::registry<Motion>.components[i];

		// Find all targets whose bounding box intersects with the circle
		float dist = distance(centerOfInstigator,
													getClosestPointOnBoundingBox(centerOfInstigator, motion));
		if (dist <= radius)
		{
			// Targets will be sorted based on the distance from the targets ground/foot
			// position to the instigator's ground/foot position
			float distanceForSorting = distance(instigatorPosition, motion.position);

			entities.emplace_back(entity, distanceForSorting);
		}
	}

	return getSortedEntities(entities);
}

std::vector<ECS::Entity> ConicalProvider::getEntities(ECS::Entity instigator,
																											vec2 targetPosition)
{
	// Pairs of entities and their angular distance
	std::vector<std::pair<ECS::Entity, float>> entities;

	// We should search in a cone that starts at the center of the instigator
	vec2 centerOfInstigator = getCenterOfEntity(instigator);
	vec2 instigatorToTarget = normalize(targetPosition - centerOfInstigator);

	for (int i = 0; i < ECS::registry<Motion>.entities.size(); i++)
	{
		auto entity = ECS::registry<Motion>.entities[i];
		auto& motion = ECS::registry<Motion>.components[i];

		// TODO: Update this function to check that the bounding box is within the cone
		vec2 sourceToEntity = normalize(motion.position - centerOfInstigator);

		float entityAngle = acos(dot(instigatorToTarget, sourceToEntity));
		if (entityAngle <= angle)
		{
			entities.emplace_back(entity, entityAngle);
		}
	}

	return getSortedEntities(entities);
}

std::vector<ECS::Entity> MouseClickProvider::getEntities(ECS::Entity instigator,
																												 vec2 targetPosition)
{
	// Pairs of entities and their distance to the mouse click
	std::vector<std::pair<ECS::Entity, float>> entities;

	// We should search in a circle around the mouse click position
	vec2 mouseClickPosition = targetPosition;

	// Find all targets whose bounding box intersects with the circle
	for (int i = 0; i < ECS::registry<Motion>.entities.size(); i++)
	{
		auto entity = ECS::registry<Motion>.entities[i];
		auto& motion = ECS::registry<Motion>.components[i];

		float dist = distance(mouseClickPosition,
													getClosestPointOnBoundingBox(mouseClickPosition, motion));
		if (dist <= radius)
		{
			// Targets will be sorted based on the distance from the targets ground/foot
			// position to the instigator's ground/foot position
			float distanceForSorting = distance(mouseClickPosition, motion.position);

			entities.emplace_back(entity, distanceForSorting);
		}
	}

	return getSortedEntities(entities);
}
