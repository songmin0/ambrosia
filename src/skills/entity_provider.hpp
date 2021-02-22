#pragma once
#include "skill_params.hpp"

#include "game/common.hpp"
#include "entities/tiny_ecs.hpp"

// These providers will fetch targets which we will then filter using an EntityFilter. After that, we'll apply a skill
// to those targets
class EntityProvider
{
public:
	EntityProvider() = default;
	~EntityProvider() = default;

	virtual std::vector<ECS::Entity> getEntities(const SkillParams& params) = 0;
};

// Collects all entities within a certain radius of the instigator
class CircularProvider : public EntityProvider
{
public:
	CircularProvider() = default;
	~CircularProvider() = default;

	std::vector<ECS::Entity> getEntities(const SkillParams& params) override;
};

// Collects all entities within a certain angular distance, relative to the source->target vector
class ConicalProvider : public EntityProvider
{
public:
	ConicalProvider() = default;
	~ConicalProvider() = default;

	std::vector<ECS::Entity> getEntities(const SkillParams& params) override;
};

// Collects all entities within a certain radius of the targetPosition (mouse click position). It's a duplicate
// of the CircularProvider except that it uses targetPosition instead of sourcePosition.
class MouseClickProvider : public EntityProvider
{
public:
	MouseClickProvider() = default;
	~MouseClickProvider() = default;

	std::vector<ECS::Entity> getEntities(const SkillParams& params) override;
};