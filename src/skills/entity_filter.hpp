#pragma once
#include "skill_params.hpp"

#include "game/common.hpp"
#include "entities/tiny_ecs.hpp"

#include <set>

// Takes a list of entities as input and outputs a filtered list of entities. See subclasses for usage
class EntityFilter
{
public:
	EntityFilter() = default;
	virtual ~EntityFilter() = default;

	virtual std::vector<ECS::Entity> process(const SkillParams& params, const std::vector<ECS::Entity>& entities) = 0;
};

// Removes the instigator from the entities list
class InstigatorFilter : public EntityFilter
{
public:
	InstigatorFilter() = default;
	~InstigatorFilter() override = default;

	std::vector<ECS::Entity> process(const SkillParams& params, const std::vector<ECS::Entity>& entities) override;
};

// Removes entities that don't match the collision mask
class CollisionFilter : public EntityFilter
{
public:
	CollisionFilter() = default;
	~CollisionFilter() override = default;

	std::vector<ECS::Entity> process(const SkillParams& params, const std::vector<ECS::Entity>& entities) override;
};
