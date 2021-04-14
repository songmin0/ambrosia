#include "physics.hpp"
#include "projectile.hpp"
#include "debug.hpp"

#include "entities/tiny_ecs.hpp"
#include "game/camera.hpp"
#include "game/camera_system.hpp"
#include "game/turn_system.hpp"
#include "game/game_state_system.hpp"
#include "animation/animation_components.hpp"
#include "ui/ui_entities.hpp"
#include "ai/ai.hpp"

#include <iostream>

// Returns a BoundingBox representing the entity's current bounds in world coords
BoundingBox PhysicsSystem::getBoundingBox(ECS::Entity entity, const Motion& motion)
{
	vec2 boundingBoxDimensions = abs(motion.boundingBox);
	float halfWidth = boundingBoxDimensions.x / 2.f;

	BoundingBox box;
	box.left = motion.position.x - halfWidth;
	box.right = motion.position.x + halfWidth;

	// For projectiles, `motion.position` refers to the center of the texture
	if (entity.has<ProjectileComponent>())
	{
		float halfHeight = boundingBoxDimensions.y / 2.f;
		box.top = motion.position.y - halfHeight;
		box.bottom = motion.position.y + halfHeight;
	}
	// For characters, `motion.position` refers to the bottom center (position of feet)
	else
	{
		box.top = motion.position.y - boundingBoxDimensions.y;
		box.bottom = motion.position.y;
	}

	return box;
}

bool collides(const BoundingBox& box1, const BoundingBox& box2)
{
	return (box1.left < box2.right) && (box1.right > box2.left) &&
				 (box1.top < box2.bottom) && (box1.bottom > box2.top);
}

////////////////////////////////////////////////////////////////////////////////
// PhysicsSystem

PhysicsSystem::PhysicsSystem(PathFindingSystem& pfs)
	: pathFindingSystem(pfs)
{
	impulseEventListener = EventSystem<ImpulseEvent>::instance().registerListener(
			std::bind(&PhysicsSystem::onImpulseEvent, this, std::placeholders::_1));
}

PhysicsSystem::~PhysicsSystem()
{
	if (impulseEventListener.isValid())
	{
		EventSystem<ImpulseEvent>::instance().unregisterListener(impulseEventListener);
	}
}

void PhysicsSystem::step(float elapsed_ms, vec2 window_size_in_game_units)
{
	if (!GameStateSystem::instance().inGameState()) {
		return;
	}
	// Move entities based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.
	float step_seconds = 1.0f * (elapsed_ms / 1000.f);

	for (auto entity : ECS::registry<Motion>.entities)
	{
		auto& motion = entity.get<Motion>();

		// See PhysicsSystem::blendMotionData for usage
		motion.prevPosition = motion.position;
		motion.prevAngle = motion.angle;

		// Projectiles don't use `motion.path` and don't experience friction. Their
		// path/velocity is managed by the ProjectileSystem.
		if (entity.has<ProjectileComponent>())
		{
			motion.position += step_seconds * motion.velocity;
		}
		else
		{
			// Get rid of any points that are close enough that no movement is needed
			while (!motion.path.empty() && length(motion.position - motion.path.top()) < THRESHOLD)
			{
				motion.path.pop();

				// When entity reaches end of path, the movement phase of its turn will end
				if (motion.path.empty())
				{
					FinishedMovementEvent event;
					event.entity = entity;
					EventSystem<FinishedMovementEvent>::instance().sendEvent(event);

					motion.velocity = { 0.f, 0.f };
				}
			}

			// If the are still points in the path, set the entity's velocity based on the next point
			if (!motion.path.empty())
			{
				// The position at the top of the stack is where the entity wants to go next. We will give the entity
				// velocity in order to reach that point, and we leave the point on the stack until the entity is within
				// the threshold distance.
				vec2 desiredPos = motion.path.top();

				// Set velocity based on the desired direction of travel
				motion.velocity = normalize(desiredPos - motion.position) * DEFAULT_SPEED;
			}
			else
			{
				if (length(motion.velocity) > 0.f)
				{
					// Apply friction in x and y directions
					applyFriction(motion.velocity.x, step_seconds);
					applyFriction(motion.velocity.y, step_seconds);
				}
			}

			// Calculate next position
			vec2 newPosition = motion.position + (motion.velocity * step_seconds);

			// Only move the entity to the next position if it's a walkable point
			if (pathFindingSystem.isWalkablePoint(entity, newPosition))
			{
				motion.position = newPosition;
			}
		}

		// Camera follows the moving entity
		if ((entity.has<PlayerComponent>() || entity.has<AISystem::MobComponent>()) && ECS::registry<CameraDelayedMoveComponent>.entities.empty()) {
			// Move camera to the entity's position if entity is out of view and is moving
			if (!CameraSystem::isPositionInView(motion.position, window_size_in_game_units) && (motion.velocity.x != 0 && motion.velocity.y != 0)) {
				CameraSystem::viewPosition(motion.position, window_size_in_game_units);
			}
			// Move camera to follow moving entity
			CameraSystem::moveCamera(step_seconds * motion.velocity, window_size_in_game_units);
		}
		else if (entity.has<ProjectileComponent>() && !entity.has<AmbrosiaProjectileComponent>()) {
			CameraSystem::viewPosition(motion.position, window_size_in_game_units);
		}

		// Position active arrow above active entity
		if (entity.has<TurnSystem::TurnComponentIsActive>()) {
			for (auto activeArrow : ECS::registry<ActiveArrow>.entities) {
				auto& offset = activeArrow.get<ActiveArrow>().offset;
				activeArrow.get<Motion>().position = motion.position + offset;
			}
		}

		//If the entity also has a stats component then move their HP bar
		if (entity.has<StatsComponent>()) {
			auto& statsComp = ECS::registry<StatsComponent>.get(entity);
			//make sure the stats component's hpbar has a motion component and is valid
			if (statsComp.healthBar.has<Motion>()) {
				auto& hpBarMotion = ECS::registry<Motion>.get(statsComp.healthBar);
				auto& hpBar = ECS::registry<HPBar>.get(statsComp.healthBar);
				hpBarMotion.position = motion.position + hpBar.offset;
			}
		}

	}

	// Visualization for debugging the position and scale of objects
	if (DebugSystem::in_debug_mode)
	{
		int numEntities = ECS::registry<Motion>.entities.size();

		for (int i = 0; i < numEntities; i++)
		{
			auto& entity = ECS::registry<Motion>.entities[i];
			auto& motion = ECS::registry<Motion>.components[i];

			if (entity.has<DebugComponent>())
			{
				continue;
			}

			// Draw this entity's path using dotted lines
			DebugSystem::createPath(motion.path);

			// Calculate the entity's current bounds
			BoundingBox box = getBoundingBox(entity, motion);
			vec2 boxDimensions = box.size();

			// Draw the debug box (prevent drawing boxes that would be way too small)
			constexpr float MIN_DIMENSION = 1.f;
			if (boxDimensions.x > MIN_DIMENSION && boxDimensions.y > MIN_DIMENSION)
			{
				DebugSystem::createBox(box.center(), box.size());
			}
		}
	}


	// Check for collisions between projectiles and all moving entities
	for (auto projectileEntity : ECS::registry<ProjectileComponent>.entities)
	{
		if (!projectileEntity.has<Motion>())
		{
			continue;
		}
		auto& projectileMotion = projectileEntity.get<Motion>();

		// Calculate the current bounds for the projectile
		BoundingBox projectileBoundingBox = getBoundingBox(projectileEntity, projectileMotion);

		// Check whether the projectile collides with any Motion entities
		for (auto targetEntity : ECS::registry<Motion>.entities)
		{
			// Ignore if targetEntity is dead or is a projectile itself
			if (targetEntity.has<DeathTimer>() || targetEntity.has<ProjectileComponent>())
			{
				continue;
			}
			auto& targetMotion = targetEntity.get<Motion>();

			// Calculate the current bounds for the targetEntity
			BoundingBox targetBoundingBox = getBoundingBox(targetEntity, targetMotion);

			// Check for collision
			if (collides(projectileBoundingBox, targetBoundingBox))
			{
				// Log the collision
				ECS::registry<Collision>.emplaceWithDuplicates(projectileEntity, targetEntity);
			}
		}
	}
}

void PhysicsSystem::blendMotionData(float alpha)
{
	for (auto& motion : ECS::registry<Motion>.components)
	{
		// Blend prev and curr position unless prev is uninitialized
		motion.renderPosition = motion.prevPosition == vec2(FLOAT_MIN) ?
														motion.position :
														mix(motion.prevPosition, motion.position, alpha);

		// Blend prev and curr angle unless prev is uninitialized
		motion.renderAngle = motion.prevAngle == FLOAT_MIN ?
												 motion.angle :
												 mix(motion.prevAngle, motion.angle, alpha);
	}
}

PhysicsSystem::Collision::Collision(ECS::Entity& other)
{
	this->other = other;
}

void PhysicsSystem::applyFriction(float& speed, float step_seconds)
{
	constexpr float COEFFICIENT_OF_FRICTION = 0.4f;
	constexpr float GRAVITATIONAL_FORCE = 300.f;

	// acceleration = F/m = μmg/m = μg
	constexpr float frictionAccel = COEFFICIENT_OF_FRICTION * GRAVITATIONAL_FORCE;

	// Speed reduction due to friction
	float deltaSpeed = frictionAccel * step_seconds;

	// Perform the speed reduction
	if (speed < 0.f)
	{
		speed = std::min(0.f, speed + deltaSpeed);
	}
	else
	{
		speed = std::max(0.f, speed - deltaSpeed);
	}
}

void PhysicsSystem::onImpulseEvent(const ImpulseEvent& event)
{
	auto entity = event.entity;

	assert(entity.has<Motion>());
	auto& motion = entity.get<Motion>();

	// delta velocity = impulse / mass
	motion.velocity += event.impulse / motion.mass;
}
