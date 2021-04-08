#pragma once
#include "animation_system.hpp"
#include "animation_components.hpp"
#include "game/game_state_system.hpp"

AnimationSystem::AnimationSystem()
{
	performSkillListener = EventSystem<PerformActiveSkillEvent>::instance().registerListener(
		std::bind(&AnimationSystem::onPerformSkillEvent, this, std::placeholders::_1));

	prepForNextMapListener = EventSystem<PrepForNextMapEvent>::instance().registerListener(
			std::bind(&AnimationSystem::onPrepForNextMapEvent, this, std::placeholders::_1));
}

AnimationSystem::~AnimationSystem()
{
	if (performSkillListener.isValid())
	{
		EventSystem<PerformActiveSkillEvent>::instance().unregisterListener(performSkillListener);
	}
	if (prepForNextMapListener.isValid())
	{
		EventSystem<PrepForNextMapEvent>::instance().unregisterListener(prepForNextMapListener);
	}
}

void AnimationSystem::updateOrientation(Motion& motion, const vec2 direction)
{
	// orientation should always be -1 or 1
	assert(abs(motion.orientation) == 1);

	// orientation check
	// left <--
	if (direction.x < 0)
	{
		// flip if they're facing right
		if (motion.scale.x * motion.orientation > 0)
		{
			motion.scale.x *= -1;
		}
	}
	// right -->
	else
	{
		// flip if they're facing left
		if (motion.scale.x * motion.orientation < 0)
		{
			motion.scale.x *= -1;
		}
	}
}

// call this every frame
void AnimationSystem::step()
{
	// for each Animation component...
	for (auto& entity : ECS::registry<AnimationsComponent>.entities)
	{
		if (!entity.has<Motion>())
		{
			continue;
		}

		auto& anims = entity.get<AnimationsComponent>();
		// get the data for the current animation
		std::shared_ptr<AnimationData>& currAnim = anims.currAnimData;

		// if the anim is waiting for frame delay, decrement the delay timer and don't do anything else
		if (currAnim->delayTimer > 0)
		{
			currAnim->delayTimer--;
			continue;
		}

		///////////////////////////////////
				/// else it's time to animate /////
			///////////////////////////////////

		// Check if we should switch between move and idle animations
		checkAnimation(entity);

		// reset the timer
		currAnim->delayTimer = currAnim->delay;

		//calculate it's current frame...
		if (!currAnim->isCycle)
		{
			// if we're on the last frame and we don't cycle...
			if (currAnim->currFrame >= currAnim->numFrames - 1)
			{
				// for all anims that don't cycle except defeat, try to return to idle
				if (anims.currentAnim != AnimationType::DEFEAT)
				{
					anims.changeAnimation(AnimationType::IDLE);
				}
			}
			else
			{
				currAnim->currFrame++;
			}
		}
		else
		{
			// the animation loops, increment and calculate cycle
			currAnim->currFrame++;
			currAnim->currFrame = currAnim->currFrame % currAnim->numFrames;
			// sanity check, if numFrames is 5, and currFrame is 5, then we want to loop back to frame 0
			// numFrames is like "length" of an array (it's 1-based) but currFrames is like "index" of an array (it's 0-based)
			// hence currAnim.currFrame should always stay between 0:(numFrames-1)
		}
	}
};

void AnimationSystem::checkAnimation(ECS::Entity& entity)
{
	auto& motion = entity.get<Motion>();
	auto& anim = entity.get<AnimationsComponent>();

	// only check move and idle animations
	if (anim.currentAnim != AnimationType::IDLE && anim.currentAnim != AnimationType::MOVE)
	{
		return;
	}

	if (abs(motion.velocity.x) > 5.0 || abs(motion.velocity.y) > 5.0)
	{
		anim.changeAnimation(AnimationType::MOVE);
		updateOrientation(motion, motion.velocity);
	}
	else
	{
		anim.changeAnimation(AnimationType::IDLE);
	}
}

void AnimationSystem::onPerformSkillEvent(const PerformActiveSkillEvent& event)
{
	auto entity = event.entity;

	if (entity.has<SkillComponent>())
	{
		auto& motion = entity.get<Motion>();
		updateOrientation(motion, event.target - motion.position);

		if (entity.has<AnimationsComponent>())
		{
			auto& anim = entity.get<AnimationsComponent>();
			auto& skillComponent = entity.get<SkillComponent>();
			if (skillComponent.getActiveSkillType() != SkillType::NONE)
			{
				std::shared_ptr<Skill> activeSkill = skillComponent.getActiveSkill();
				anim.changeAnimation(activeSkill->getAnimationType());
			}
		}
	}
}

void AnimationSystem::onPrepForNextMapEvent(const PrepForNextMapEvent& event)
{
	auto entity = event.entity;
	if (entity.has<AnimationsComponent>())
	{
		entity.get<AnimationsComponent>().changeAnimation(AnimationType::IDLE);
	}
}
