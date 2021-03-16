#include "entity_filter.hpp"

bool InstigatorFilter::process(ECS::Entity entity)
{
	return instigator.id != entity.id;
}

bool CollisionFilter::process(ECS::Entity entity)
{
	return entity.has<Motion>() && (entity.get<Motion>().colliderType & collidesWith);
}

bool MaxTargetsFilter::process(ECS::Entity entity)
{
	targetCount++;
	return targetCount <= maxTargets;
}

void MaxTargetsFilter::reset()
{
	targetCount = 0;
}
