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

	void step(float elapsed_ms);

private:
	struct LaunchRequest
	{
		ECS::Entity instigator;
		vec2 targetPosition;
		ProjectileParams params;
	};

	void updateLinearTrajectory(float elapsed_s, ECS::Entity projEntity, ProjectileComponent& projComponent);
	void updateBoomerangTrajectory(float elapsed_s, ECS::Entity projEntity, ProjectileComponent& projComponent);

	void launchProjectile(LaunchRequest& request);

	void onLaunchBulletEvent(const LaunchBulletEvent& event);
	void onLaunchBoneEvent(const LaunchBoneEvent& event);

	EventListenerInfo launchBulletListener;
	EventListenerInfo launchBoneListener;

	std::vector<LaunchRequest> launchRequests;
};

