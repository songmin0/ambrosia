#pragma once

#include "game/common.hpp"
#include "entities/tiny_ecs.hpp"

#include <set>

class EntityFilter
{
public:
	EntityFilter() = default;
	virtual ~EntityFilter() = default;

	// The `process` function returns true if the entity should be kept and false
	// if the entity should be filtered out.
	virtual bool process(ECS::Entity entity) = 0;

	virtual void reset() {}
};

// Prevents the instigator from being targeted
class InstigatorFilter : public EntityFilter
{
public:
	InstigatorFilter(ECS::Entity instigator)
		: instigator(instigator)
	{}

	~InstigatorFilter() override = default;

	bool process(ECS::Entity entity) override;

private:
	ECS::Entity instigator;
};

// Prevents targeting entities that don't match the collision mask
class CollisionFilter : public EntityFilter
{
public:
	CollisionFilter(CollisionGroup collidesWith)
		: collidesWith(collidesWith)
	{}

	~CollisionFilter() override = default;

	bool process(ECS::Entity entity) override;

private:
	CollisionGroup collidesWith;
};

// This filter helps us to control whether a skill should be applied to only the
// closest entity or to multiple entities. It counts how many times it gets
// called, and once the count is over the maximum, all subsequent entities will
// be filtered out. Make sure to put this as the last filter in your list of
// filters.
class MaxTargetsFilter : public EntityFilter
{
public:
	MaxTargetsFilter(int maxTargets)
		: maxTargets(maxTargets)
		, targetCount(0)
	{}

	~MaxTargetsFilter() override = default;

	bool process(ECS::Entity entity) override;
	void reset() override;

private:
	int maxTargets;
	int targetCount;
};
