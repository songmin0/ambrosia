#pragma once

#include "game/common.hpp"
#include "entities/tiny_ecs.hpp"

// These providers will fetch targets which we will then filter using an
// EntityFilter. After that, we'll apply a skill to those targets
class EntityProvider
{
public:
	EntityProvider() = default;
	virtual ~EntityProvider() = default;

	virtual std::vector<ECS::Entity> getEntities(ECS::Entity instigator,
																							 vec2 targetPosition) = 0;
};

// Collects all entities (that have a Motion component) on the map
class AllEntitiesProvider : public EntityProvider
{
public:
	AllEntitiesProvider() = default;
	~AllEntitiesProvider() override = default;

	std::vector<ECS::Entity> getEntities(ECS::Entity instigator,
																			 vec2 targetPosition) override;
};

// Collects all entities within a certain radius of the instigator
class CircularProvider : public EntityProvider
{
public:
	CircularProvider(float radius)
		: radius(radius)
	{}

	~CircularProvider() override = default;

	std::vector<ECS::Entity> getEntities(ECS::Entity instigator,
																			 vec2 targetPosition) override;

	float getRadius();

private:
	float radius;
};

// Collects all entities within a certain angular distance, relative to the
// source->target vector
class ConicalProvider : public EntityProvider
{
public:
	ConicalProvider(float angle)
		: angle(angle)
	{}

	~ConicalProvider() override = default;

	std::vector<ECS::Entity> getEntities(ECS::Entity instigator,
																			 vec2 targetPosition) override;

private:
	float angle;
};

// Collects all entities within a certain radius of the targetPosition
// (mouse click position). It's a duplicate of the CircularProvider except
// that it uses targetPosition instead of sourcePosition.
class MouseClickProvider : public EntityProvider
{
public:
	MouseClickProvider(float radius)
		: radius(radius)
	{}

	~MouseClickProvider() override = default;

	std::vector<ECS::Entity> getEntities(ECS::Entity instigator,
																			 vec2 targetPosition) override;

	float getRadius();

private:
	float radius;
};