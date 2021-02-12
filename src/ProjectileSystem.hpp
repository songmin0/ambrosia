#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "Projectile.hpp"
#include "EventSystem.hpp"
#include "Events.hpp"

class ProjectileSystem
{
public:
	ProjectileSystem();
	~ProjectileSystem();

	void Step(float elapsed_ms);

private:
	struct LaunchRequest
	{
		ECS::Entity instigator;
		vec2 targetPosition;
		ProjectileParams params;
	};

	void UpdateLinearTrajectory(float elapsed_s, ECS::Entity projEntity, ProjectileComponent& projComponent);
	void UpdateBoomerangTrajectory(float elapsed_s, ECS::Entity projEntity, ProjectileComponent& projComponent);

	void LaunchProjectile(LaunchRequest& request);

	void OnLaunchBulletEvent(const LaunchBulletEvent& event);
	void OnLaunchBoneEvent(const LaunchBoneEvent& event);

	EventListenerInfo launchBulletListener;
	EventListenerInfo launchBoneListener;

	std::vector<LaunchRequest> launchRequests;
};

