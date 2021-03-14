#include "physics.hpp"
#include "projectile.hpp"
#include "debug.hpp"

#include "entities/tiny_ecs.hpp"
#include "game/camera.hpp"
#include "game/camera_system.hpp"
#include "game/turn_system.hpp"
#include "animation/animation_components.hpp"
#include "ui/ui_entities.hpp"
#include <ai/ai.hpp>

#include <iostream>

// Returns a BoundingBox representing the entity's current bounds in world coords
BoundingBox getBoundingBox(ECS::Entity entity, const Motion& motion)
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

void PhysicsSystem::step(float elapsed_ms, vec2 window_size_in_game_units)
{
	// Move entities based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.
	
	for (auto entity : ECS::registry<Motion>.entities)
	{
		auto& motion = entity.get<Motion>();

		// Projectiles don't use `motion.path`, so they need to skip this block of code. Their
		// path/velocity is managed by the ProjectileSystem.
		if (!entity.has<ProjectileComponent>())
		{
			// Get rid of any points that are close enough that no movement is needed
			const float THRESHOLD = 3.f;
			while (!motion.path.empty() && length(motion.position - motion.path.top()) < THRESHOLD)
			{
				motion.path.pop();

				// When entity reaches end of path, the movement phase of its turn will end
				if (motion.path.empty())
				{
					FinishedMovementEvent event;
					event.entity = entity;
					EventSystem<FinishedMovementEvent>::instance().sendEvent(event);
				}
			}

			// If no path, make sure velocity is zero
			if (motion.path.empty())
			{
				motion.velocity = {0.f, 0.f};
			}
			else
			{
				const float DEFAULT_SPEED = 100.f; // TEMPORARY

				// The position at the top of the stack is where the entity wants to go next. We will give the entity
				// velocity in order to reach that point, and we leave the point on the stack until the entity is within
				// the threshold distance.
				vec2 desiredPos = motion.path.top();

				// Set velocity based on the desired direction of travel
				motion.velocity = normalize(desiredPos - motion.position) * DEFAULT_SPEED;
			}
		}

		float step_seconds = 1.0f * (elapsed_ms / 1000.f);
		motion.position += step_seconds * motion.velocity;

		// Camera follows the moving entity
		if ((entity.has<PlayerComponent>() || entity.has<AISystem::MobComponent>()) && ECS::registry<CameraDelayedMoveComponent>.entities.empty()) {
			// Move camera to the entity's position if entity is out of view and is moving
			if (!CameraSystem::isPositionInView(motion.position, window_size_in_game_units) && (motion.velocity.x != 0 && motion.velocity.y != 0)) {
				CameraSystem::viewPosition(motion.position, window_size_in_game_units);
			}
			// Move camera to follow moving entity
			CameraSystem::moveCamera(step_seconds * motion.velocity, window_size_in_game_units);
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

	// Check for collisions between all moving entities
	auto& motion_container = ECS::registry<Motion>;
	// for (auto [i, motion_i] : enumerate(motion_container.components)) // in c++ 17 we will be able to do this instead of the next three lines
	for (unsigned int i=0; i<motion_container.components.size(); i++)
	{
		Motion& motion_i = motion_container.components[i];
		ECS::Entity entity_i = motion_container.entities[i];

		// Don't check for collisions if entity_i is dead
		if (entity_i.has<DeathTimer>())
		{
			continue;
		}

		// Calculate the current bounds for entity_i
		BoundingBox boundingBox_i = getBoundingBox(entity_i, motion_i);

		for (unsigned int j=i+1; j<motion_container.components.size(); j++)
		{
			Motion& motion_j = motion_container.components[j];
			ECS::Entity entity_j = motion_container.entities[j];

			// Don't check for collisions if entity_j is dead
			if (entity_j.has<DeathTimer>())
			{
				continue;
			}

			// Make sure these entities are allowed to collide with one another
			if (motion_i.collidesWith & motion_j.colliderType ||
					motion_j.collidesWith & motion_i.colliderType)
			{
				// Calculate the current bounds for entity_j
				BoundingBox boundingBox_j = getBoundingBox(entity_j, motion_j);

				// Check for collision
				if (collides(boundingBox_i, boundingBox_j))
				{
					// Log the collisions
					ECS::registry<Collision>.emplaceWithDuplicates(entity_i, entity_j);
					ECS::registry<Collision>.emplaceWithDuplicates(entity_j, entity_i);
				}
			}
		}
	}
}

PhysicsSystem::Collision::Collision(ECS::Entity& other)
{
	this->other = other;
}
