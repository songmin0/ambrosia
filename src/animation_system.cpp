#pragma once
#include "animation_components.hpp"
#include "animation_system.hpp"

// call this every frame
void AnimationSystem::step()
{
	// for each Animation component...
	for (auto& entity : ECS::registry<AnimationsComponent>.entities)
	{
		auto& anims = entity.get<AnimationsComponent>();
		// get the data for the current animation
		AnimationData& currAnim = anims.currAnimData;

		// if the anim is waiting for frame delay, decrement the delay timer and don't do anything else
		if (currAnim.delay_timer > 0)
		{
			currAnim.delay_timer--;
			continue;
		}

		///////////////////////////////////
	   /// else it's time to animate /////
	  ///////////////////////////////////

		// Check if we should switch between move and idle animations
		CheckAnimation(entity);

	   // reset the timer
		currAnim.delay_timer = currAnim.delay;

		//calculate it's current frame...
		if (!currAnim.cycle)
		{
			// if we're on the last frame and we don't cycle...
			if (currAnim.currFrame >= currAnim.numFrames - 1)
			{
				// for all anims that don't cycle except defeat, try to return to idle
				anims.ChangeAnimation(AnimationType::IDLE);
			}
			else
			{
				currAnim.currFrame++;
			}
		}
		else
		{
			// the animation loops, increment and calculate cycle
			currAnim.currFrame++;
			currAnim.currFrame = currAnim.currFrame % currAnim.numFrames;
			// sanity check, if numFrames is 5, and currFrame is 5, then we want to loop back to frame 0
			// numFrames is like "length" of an array (it's 1-based) but currFrames is like "index" of an array (it's 0-based)
			// hence currAnim.currFrame should always stay between 0:(numFrames-1)
		}
	}
};

void AnimationSystem::CheckAnimation(ECS::Entity& entity)
{
	auto& motion = entity.get<Motion>();
	auto& anim = entity.get<AnimationsComponent>();
	if (abs(motion.velocity.x) > 5.0 || abs(motion.velocity.y) > 5.0)
	{
		anim.ChangeAnimation(AnimationType::MOVE);

		// orientation check
		if (motion.velocity.x < 0)
		{
			if (motion.scale.x > 0)
			{
				motion.scale.x *= -1;
			}
		}
		else
		{
			if (motion.scale.x < 0)
			{
				motion.scale.x *= -1;
			}
		}
	}
	else
	{
		anim.ChangeAnimation(AnimationType::IDLE);
	}
}