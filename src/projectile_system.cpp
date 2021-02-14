#include "projectile_system.hpp"
#include "projectile.hpp"
#include "render.hpp"
#include "animation_components.hpp"

#include <functional>


ProjectileSystem::ProjectileSystem()
{
	launchBulletListener = EventSystem<LaunchBulletEvent>::instance().registerListener(
			std::bind(&ProjectileSystem::onLaunchBulletEvent, this, std::placeholders::_1));

	launchBoneListener = EventSystem<LaunchBoneEvent>::instance().registerListener(
			std::bind(&ProjectileSystem::onLaunchBoneEvent, this, std::placeholders::_1));
}

ProjectileSystem::~ProjectileSystem()
{
	if (launchBulletListener.isValid())
	{
		EventSystem<LaunchBulletEvent>::instance().unregisterListener(launchBulletListener);
	}

	if (launchBoneListener.isValid())
	{
		EventSystem<LaunchBoneEvent>::instance().unregisterListener(launchBoneListener);
	}
}

void ProjectileSystem::step(float elapsed_ms)
{
	const float elapsed_s = elapsed_ms / 1000.f;

	// Service launch requests
	for (int i = launchRequests.size() - 1; i >= 0; i--)
	{
		LaunchRequest& request = launchRequests[i];

		request.params.delay -= elapsed_s;

		if (request.params.delay <= 0)
		{
			launchProjectile(request);
			launchRequests.erase(launchRequests.begin() + i);
		}
	}

	// Update the projectile velocities and remove any projectiles that have finished
	for (int i = ECS::registry<ProjectileComponent>.entities.size() - 1; i >= 0; i--)
	{
		auto& projEntity = ECS::registry<ProjectileComponent>.entities[i];
		auto& projComponent = projEntity.get<ProjectileComponent>();

		projComponent.timeSinceLaunch += elapsed_s;

		// Projectile handling based on trajectory type
		if (projComponent.params.trajectory == Trajectory::LINEAR)
		{
			updateLinearTrajectory(elapsed_s, projEntity, projComponent);
		}
		else
		{
			updateBoomerangTrajectory(elapsed_s, projEntity, projComponent);
		}

		// Remove projectile if needed
		if (projComponent.phase == Phase::END)
		{
			ECS::registry<ProjectileComponent>.removeAllComponentsOf(projEntity);
		}
	}
}

void ProjectileSystem::updateLinearTrajectory(float elapsed_s, ECS::Entity projEntity, ProjectileComponent& projComponent)
{
	auto& projMotion = projEntity.get<Motion>();

	// For linear trajectory, we only need to set the velocity at the very beginning
	if (projComponent.phase == Phase::INIT)
	{
		projMotion.velocity = normalize(projComponent.targetPosition - projMotion.position) * projComponent.params.launchSpeed;
		projComponent.phase = Phase::PHASE1;
	}

	// Rotate the projectile
	projMotion.angle += projComponent.params.rotationSpeed * elapsed_s;

	// Check if the projectile has reached the target
	const float TEMP_THRESHOLD = 5.f;
	if (length(projComponent.targetPosition - projMotion.position) < TEMP_THRESHOLD)
	{
		projComponent.phase = Phase::END;
	}
}

void ProjectileSystem::updateBoomerangTrajectory(float elapsed_s, ECS::Entity projEntity, ProjectileComponent& projComponent)
{
	// For the boomerang, the velocity gets updated in every tick, so there's no special "launch" logic. Just put it in
	// phase 1 immediately
	if (projComponent.phase == Phase::INIT)
	{
		projComponent.phase = Phase::PHASE1;
	}

	auto& projMotion = projEntity.get<Motion>();
	vec2 launchPosition = projComponent.sourcePosition;

	// Vector from source to target
	vec2 sourceToTarget = projComponent.targetPosition - launchPosition;

	// Direction from instigator to target
	vec2 directionToTarget = normalize(sourceToTarget);

	// The total distance travelled so far
	float distanceTravelled = length(projMotion.position - launchPosition);

	// Proportion of the total distance that has been travelled so far
	float proportionTravelled = distanceTravelled / length(sourceToTarget);

	// A vector perpendicular to the linear path (the linear path from the source to the target)
	vec2 perpendicular = normalize(vec2(sourceToTarget.y, -sourceToTarget.x));

	// While the projectile is travelling from the source to the target, this offset will give it a curved trajectory
	vec2 perpendicularOffset = perpendicular * std::sin(proportionTravelled * PI) * 200.f;

	// Calculate where the projectile would be right now, if it were travelling on a straight-line path to the target
	vec2 linearDesiredPosition = launchPosition + (directionToTarget * projComponent.params.launchSpeed * projComponent.timeSinceLaunch);

	// This is where the projectile should really be
	vec2 actualDesiredPosition = linearDesiredPosition + perpendicularOffset;

	// Based on the desired position above, we can calculate the velocity required to get the projectile to that position
	vec2 velocity = normalize(actualDesiredPosition - projMotion.position) * projComponent.params.launchSpeed;

	// Finally, update the projectile's velocity. The physics system will handle the actual movement
	projMotion.velocity = velocity;

	// Rotate the projectile
	projMotion.angle += projComponent.params.rotationSpeed * elapsed_s;

	if (proportionTravelled > 1.f)
	{
		// The projectile has reached the target, so we turn it around to go back to the instigator
		if (projComponent.phase == Phase::PHASE1)
		{
			projComponent.phase = Phase::PHASE2;
			std::swap(projComponent.sourcePosition, projComponent.targetPosition);
			projComponent.timeSinceLaunch = 0.f;
		}
		else
		{
			// In this case, the projectile is in phase 2 (i.e., travelling back toward the instigator), and it reaches the
			// instigator, so it's time to remove the projectile
			projComponent.phase = Phase::END;
		}
	}
}

void ProjectileSystem::launchProjectile(LaunchRequest& request)
{
	auto entity = ECS::Entity();

	// Create rendering primitives
	ShadedMesh& resource = cacheResource(request.params.spritePath);
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, spritePath(request.params.spritePath + ".png"), "textured");
	}

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	entity.emplace<ShadedMeshRef>(resource);

	// Projectile component setup
	auto& projComponent = entity.emplace<ProjectileComponent>();
	projComponent.instigator = request.instigator;
	projComponent.sourcePosition = request.instigator.get<Motion>().position + request.params.launchOffset;
	projComponent.targetPosition = request.targetPosition;
	projComponent.params = request.params;

	// Setting initial motion values
	Motion& motion = entity.emplace<Motion>();
	motion.position = projComponent.sourcePosition;
	motion.angle = 0.f;
	motion.velocity = {0.f, 0.f};
	motion.scale = request.params.spriteScale;
	motion.boundingBox = motion.scale * vec2(resource.texture.size.x, resource.texture.size.y);
}

void ProjectileSystem::onLaunchBulletEvent(const LaunchBulletEvent& event)
{
	LaunchRequest request;
	request.instigator = event.instigator;
	request.targetPosition = event.targetPosition;
	request.params = ProjectileParams::createBulletParams();

	launchRequests.push_back(request);
}

void ProjectileSystem::onLaunchBoneEvent(const LaunchBoneEvent& event)
{
	LaunchRequest request;
	request.instigator = event.instigator;
	request.targetPosition = event.targetPosition;
	request.params = ProjectileParams::createBoneParams();

	launchRequests.push_back(request);

	// Activating the animation here for the time being. Added a delay to the projectile launch so that it syncs
	// up with this attack animation
	auto& anim = request.instigator.get<AnimationsComponent>();
	anim.changeAnimation(AnimationType::ATTACK3);
}
