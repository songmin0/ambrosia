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

	if (type == ProjectileType::BLUEBERRY)
	{
		params.spritePath = "players/chia/chia-blueberry";
		params.spriteScale = vec2(0.7f);
		params.launchOffset = {0.f, -75.f};
		params.launchSpeed = 500.f;
		params.rotationSpeed = 10.f;
		params.trajectory = Trajectory::LINEAR;
	}
	else if (type == ProjectileType::BONE)
	{
		params.spritePath = "players/raoul/raoul-bone";
		params.spriteScale = {0.5f, 0.5f};
		params.launchOffset = {0.f, -75.f};
		params.launchSpeed = 500.f;
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
		params.rotationSpeed = 50.f;
		params.trajectory = Trajectory::LINEAR;
	}
	else if (type == ProjectileType::HEAL_ORB)
	{
		params.spritePath = "enemies/milk/heal-orb";
		params.spriteScale = vec2(1.f);
		params.launchOffset = { 0.f, -75.f };
		params.launchSpeed = 300.f;
		params.rotationSpeed = 50.f;
		params.trajectory = Trajectory::LINEAR;
	}
	else if (type == ProjectileType::PEPPER)
	{
		params.spritePath = "enemies/saltnpepper/pepper";
		params.spriteScale = vec2(1.f);
		params.launchOffset = { 0.f, -175.f };
		params.launchSpeed = 400.f;
		params.rotationSpeed = 10.f;
		params.trajectory = Trajectory::LINEAR;
	}
	else if (type == ProjectileType::SALT)
	{
		params.spritePath = "enemies/saltnpepper/salt";
		params.spriteScale = vec2(1.f);
		params.launchOffset = { 0.f, -175.f };
		params.launchSpeed = 400.f;
		params.rotationSpeed = 10.f;
		params.trajectory = Trajectory::LINEAR;
	}
	else if (type == ProjectileType::DRUMSTICK)
	{
		params.spritePath = "enemies/chicken/drumstick";
		params.spriteScale = vec2(1.f);
		params.launchOffset = { -60.f, -275.f };
		params.launchSpeed = 500.f;
		params.rotationSpeed = 5.f;
		params.trajectory = Trajectory::BOOMERANG;
	}
	else if (type == ProjectileType::AMBROSIA_ICON)
	{
		params.spritePath = "ambrosia-icon";
		params.spriteScale = vec2(0.7f);
		params.launchOffset = vec2(0.f);
		params.launchSpeed = 700.f;
		params.rotationSpeed = 0.f;
		params.trajectory = Trajectory::CURVED;
	}

	return params;
}

ProjectileComponent::ProjectileComponent()
	: sourcePosition(0.f, 0.f)
	, targetPosition(0.f, 0.f)
	, timeSinceLaunch(0.f)
	, phase(Phase::INIT)
	, callback(nullptr)
	, targetPositionProvider(nullptr)
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
	if (entityHandler)
	{
		entityHandler->process(instigator, entity);
	}
}
