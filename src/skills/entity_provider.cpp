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
		float boundingBoxHalfWidth = boundingBox.x / 2.f;

		float left = targetMotion.position.x - boundingBoxHalfWidth;
		float right = targetMotion.position.x + boundingBoxHalfWidth;
		float top = targetMotion.position.y - boundingBox.y;
		float bottom = targetMotion.position.y;

		vec2 closestPoint;
		closestPoint.x = clamp(source.x, left, right);
		closestPoint.y = clamp(source.y, top, bottom);

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

std::vector<ECS::Entity> AllEntitiesProvider::getEntities(ECS::Entity instigator,
																													vec2 targetPosition)
{
	return ECS::registry<Motion>.entities;
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
			// Targets will be sorted based on the distance from the target's ground/foot
			// position to the instigator's ground/foot position
			float distanceForSorting = distance(instigatorPosition, motion.position);

			entities.emplace_back(entity, distanceForSorting);
		}
	}

	return getSortedEntities(entities);
}

float CircularProvider::getRadius()
{
	return radius;
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

	for (int i = 0; i < ECS::registry<Motion>.entities.size(); i++)
	{
		auto entity = ECS::registry<Motion>.entities[i];
		auto& motion = ECS::registry<Motion>.components[i];

		// Find all targets whose bounding box intersects with the circle
		float dist = distance(mouseClickPosition,
													getClosestPointOnBoundingBox(mouseClickPosition, motion));
		if (dist <= radius)
		{
			// Targets will be sorted based on the distance from the target's center
			// to the mouse click position
			float distanceForSorting = distance(mouseClickPosition, getCenterOfEntity(entity));

			entities.emplace_back(entity, distanceForSorting);
		}
	}

	return getSortedEntities(entities);
}

float MouseClickProvider::getRadius()
{
	return radius;
}
