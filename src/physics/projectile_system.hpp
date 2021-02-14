#pragma once
#include "projectile.hpp"

#include "entities/tiny_ecs.hpp"
#include "game/common.hpp"
#include "game/event_system.hpp"
#include "game/events.hpp"

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

