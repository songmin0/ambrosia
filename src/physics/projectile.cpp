#include "projectile.hpp"

ProjectileParams::ProjectileParams()
	: spritePath("")
	, spriteScale(0.f, 0.f)
	, launchOffset(0.f, 0.f)
	, launchSpeed(0.f)
	, rotationSpeed(0.f)
	, damage(0.f)
	, trajectory(Trajectory::LINEAR)
{}

ProjectileParams ProjectileParams::create(ProjectileType type, float damage)
{
	ProjectileParams params;

	if (type == ProjectileType::BULLET)
	{
		params.spritePath = "placeholder";
		params.spriteScale = {1.f, 1.f};
		params.launchOffset = {0.f, -75.f};
		params.launchSpeed = 300.f;
		params.rotationSpeed = 300.f;
		params.damage = damage;
		params.trajectory = Trajectory::LINEAR;
	}
	else if (type == ProjectileType::BONE)
	{
		params.spritePath = "players/raoul/raoul-bone";
		params.spriteScale = {0.5f, 0.5f};
		params.launchOffset = {0.f, -75.f};
		params.launchSpeed = 300.f;
		params.rotationSpeed = 200.f;
		params.damage = damage;
		params.trajectory = Trajectory::BOOMERANG;
	}
	else if (type == ProjectileType::EGG_SHELL)
	{
		params.spritePath = "enemies/egg/egg_shell";
		params.spriteScale = {0.5f, 0.5f};
		params.launchOffset = {0.f, -75.f};
		params.launchSpeed = 300.f;
		params.rotationSpeed = 200.f;
		params.damage = damage;
		params.trajectory = Trajectory::LINEAR;
	}
	else if (type == ProjectileType::DAMAGE_ORB)
	{
		params.spritePath = "enemies/milk/damage-orb";
		params.spriteScale = vec2(1.f);
		params.launchOffset = { 0.f, -75.f };
		params.launchSpeed = 300.f;
		params.rotationSpeed = 200.f;
		params.damage = damage;
		params.trajectory = Trajectory::LINEAR;
	}
	else if (type == ProjectileType::HEAL_ORB)
	{
		params.spritePath = "enemies/milk/heal-orb";
		params.spriteScale = vec2(1.f);
		params.launchOffset = { 0.f, -75.f };
		params.launchSpeed = 300.f;
		params.rotationSpeed = 200.f;
		params.damage = -damage; // hacky heal, won't prevent HP overflow
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
