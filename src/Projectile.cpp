#include "Projectile.hpp"


ProjectileParams::ProjectileParams()
	: spritePath("")
	, spriteScale(0.f, 0.f)
	, launchOffset(0.f, 0.f)
	, launchSpeed(0.f)
	, rotationSpeed(0.f)
	, delay(0.f)
	, trajectory(Trajectory::LINEAR)
{}

ProjectileParams ProjectileParams::CreateBulletParams() {

	ProjectileParams params;
	params.spritePath = "placeholder";
	params.spriteScale = {1.f, 1.f};
	params.launchOffset = {0.f, -75.f};
	params.launchSpeed = 300.f;
	params.rotationSpeed = 300.f;
	params.delay = 1.3f;
	params.trajectory = Trajectory::LINEAR;

	return params;
}

ProjectileParams ProjectileParams::CreateBoneParams() {

	ProjectileParams params;
	params.spritePath = "players/raoul/raoul-bone";
	params.spriteScale = {0.5f, 0.5f};
	params.launchOffset = {0.f, -75.f};
	params.launchSpeed = 200.f;
	params.rotationSpeed = 200.f;
	params.delay = 1.3f;
	params.trajectory = Trajectory::BOOMERANG;

	return params;
}

ProjectileComponent::ProjectileComponent()
	: sourcePosition(0.f, 0.f)
	, targetPosition(0.f, 0.f)
	, timeSinceLaunch(0.f)
	, phase(Phase::INIT)
{}