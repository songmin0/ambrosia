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
	void updateLinearTrajectory(float elapsed_s, ECS::Entity projEntity, ProjectileComponent& projComponent);
	void updateCurvedTrajectory(float elapsed_s, ECS::Entity projEntity, ProjectileComponent& projComponent);

	void onLaunchEvent(const LaunchEvent& event);

	EventListenerInfo launchListener;
};

