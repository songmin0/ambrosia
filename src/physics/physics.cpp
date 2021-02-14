#include "physics.hpp"
#include "projectile.hpp"
#include "debug.hpp"

#include "entities/tiny_ecs.hpp"
#include "game/turn_system.hpp"
#include "animation/animation_components.hpp"

#include <iostream>

// Returns the local bounding coordinates scaled by the current size of the entity 
vec2 getBoundingBox(const Motion& motion)
{
	// fabs is to avoid negative scale due to the facing direction.
	return { abs(motion.boundingBox.x), abs(motion.boundingBox.y) };
}

// This is a SUPER APPROXIMATE check that puts a circle around the bounding boxes and sees
// if the center point of either object is inside the other's bounding-box-circle. You don't
// need to try to use this technique.
//bool collides(const Motion& motion1, const Motion& motion2)
//{
//	auto dp = motion1.position - motion2.position;
//	float dist_squared = dot(dp,dp);
//	float other_r = std::sqrt(std::pow(get_bounding_box(motion1).x/2.0f, 2.f) + std::pow(get_bounding_box(motion1).y/2.0f, 2.f));
//	float my_r = std::sqrt(std::pow(get_bounding_box(motion2).x/2.0f, 2.f) + std::pow(get_bounding_box(motion2).y/2.0f, 2.f));
//	float r = max(other_r, my_r);
//	if (dist_squared < r * r)
//		return true;
//	return false;
//}

bool collides(const Motion& motion1, const Motion& motion2)
{
	auto boundingBox1 = getBoundingBox(motion1);
	auto boundingBox2 = getBoundingBox(motion2);

	bool collisionX = boundingBox1.x + motion1.position.x >= motion2.position.x &&
						motion1.position.x <= boundingBox2.x + motion2.position.x;

	bool collisionY = motion1.position.y - boundingBox1.y <= motion2.position.y &&
						motion1.position.y >= motion2.position.y - boundingBox2.y;

	bool debugEnabled = false;
	if (debugEnabled && collisionX && collisionY) {
		std::cout << "bounding box 1 x: " << boundingBox1.x;
		std::cout << "bounding box 1 y: " << boundingBox1.y;
		std::cout << "bounding box 2 x: " << boundingBox2.x;
		std::cout << "bounding box 2 y: " << boundingBox2.y;
	}
	
	return collisionX && collisionY;
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
				if (motion.path.empty() && entity.has<TurnSystem::TurnComponent>())
				{
					entity.get<TurnSystem::TurnComponent>().hasMoved = true;
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
	}

	// Visualization for debugging the position and scale of objects
	if (DebugSystem::in_debug_mode)
	{
		int numEntities = ECS::registry<Motion>.entities.size();

		for (int i = 0; i < numEntities; i++)
		{
			auto& entity = ECS::registry<Motion>.entities[i];
			auto& motion = ECS::registry<Motion>.components[i];

			vec2 position = motion.position;

			if (entity.has<AnimationsComponent>())
			{
				// For entities with an AnimationsComponent, we translate the sprite upward by half the texture size so that the
				// entity's position refers to their feet instead of the middle of the sprite. This happens in
				// RenderSystem::drawAnimatedMesh. See `transform.translate(vec2(0.f, -0.5f))`. The line of code below this
				// comment just aligns the debug lines properly for those entities.
				position.y -= motion.boundingBox.y / 2;
			}

			// draw a cross at the position of all objects
			auto scale_horizontal_line = motion.boundingBox;
			scale_horizontal_line.y *= 0.1f;
			auto scale_vertical_line = motion.boundingBox;
			scale_vertical_line.x *= 0.1f;
			DebugSystem::createLine(position, scale_horizontal_line);
			DebugSystem::createLine(position, scale_vertical_line);
		}
	}

	// Check for collisions between all moving entities
	auto& motion_container = ECS::registry<Motion>;
	// for (auto [i, motion_i] : enumerate(motion_container.components)) // in c++ 17 we will be able to do this instead of the next three lines
	for (unsigned int i=0; i<motion_container.components.size(); i++)
	{
		Motion& motion_i = motion_container.components[i];
		ECS::Entity entity_i = motion_container.entities[i];
		for (unsigned int j=i+1; j<motion_container.components.size(); j++)
		{
			Motion& motion_j = motion_container.components[j];
			ECS::Entity entity_j = motion_container.entities[j];

			if (collides(motion_i, motion_j))
			{
				// Create a collision event
				// Note, we are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity, hence, emplaceWithDuplicates
				ECS::registry<Collision>.emplaceWithDuplicates(entity_i, entity_j);
				ECS::registry<Collision>.emplaceWithDuplicates(entity_j, entity_i);
			}
		}
	}
}

PhysicsSystem::Collision::Collision(ECS::Entity& other)
{
	this->other = other;
}
