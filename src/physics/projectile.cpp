#include "projectile.hpp"

ProjectileParams::ProjectileParams()
	: spritePath("")
	, spriteScale(0.f, 0.f)
	, launchOffset(0.f, 0.f)
	, launchSpeed(0.f)
	, rotationSpeed(0.f)
	, trajectory(Trajectory::LINEAR)
{}

ProjectileParams ProjectileParams::create(ProjectileType type)
{
	assert(type != ProjectileType::INVALID);
	ProjectileParams params;

	if (type == ProjectileType::BULLET)
	{
		params.spritePath = "placeholder";
		params.spriteScale = {1.f, 1.f};
		params.launchOffset = {0.f, -75.f};
		params.launchSpeed = 300.f;
		params.rotationSpeed = 300.f;
		params.trajectory = Trajectory::LINEAR;
	}
	else if (type == ProjectileType::BONE)
	{
		params.spritePath = "players/raoul/raoul-bone";
		params.spriteScale = {0.5f, 0.5f};
		params.launchOffset = {0.f, -75.f};
		params.launchSpeed = 300.f;
		params.rotationSpeed = 200.f;
		params.trajectory = Trajectory::BOOMERANG;
	}
	else if (type == ProjectileType::EGG_SHELL)
	{
		params.spritePath = "enemies/egg/egg_shell";
		params.spriteScale = {0.5f, 0.5f};
		params.launchOffset = {0.f, -75.f};
		params.launchSpeed = 300.f;
		params.rotationSpeed = 200.f;
		params.trajectory = Trajectory::LINEAR;
	}
	else if (type == ProjectileType::DAMAGE_ORB)
	{
		params.spritePath = "enemies/milk/damage-orb";
		params.spriteScale = vec2(1.f);
		params.launchOffset = { 0.f, -75.f };
		params.launchSpeed = 300.f;
		params.rotationSpeed = 200.f;
		params.trajectory = Trajectory::LINEAR;
	}
	else if (type == ProjectileType::HEAL_ORB)
	{
		params.spritePath = "enemies/milk/heal-orb";
		params.spriteScale = vec2(1.f);
		params.launchOffset = { 0.f, -75.f };
		params.launchSpeed = 300.f;
		params.rotationSpeed = 200.f;
		params.trajectory = Trajectory::LINEAR;
	}

	return params;
}

ProjectileComponent::ProjectileComponent()
	: sourcePosition(0.f, 0.f)
	, targetPosition(0.f, 0.f)
	, timeSinceLaunch(0.f)
	, phase(Phase::INIT)
	, callback(nullptr)
{}

void ProjectileComponent::processCollision(ECS::Entity entity)
{
	// Don't process an entity more than once
	if (ignoredEntities.count(entity.id) > 0)
	{
		return;
	}
	ignoredEntities.insert(entity.id);

	// Run the entity through the filters
	for (auto& filter : entityFilters)
	{
		assert(filter);

		// If a filter returns false, that means the entity should get filtered out
		if (!filter->process(entity))
		{
			return;
		}
	}

	// Apply the entity handler
	assert(entityHandler);
	entityHandler->process(instigator, entity);
}
