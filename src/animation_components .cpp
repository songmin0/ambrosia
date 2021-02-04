#pragma once
#include "animation_components.hpp"


AnimationData::AnimationData()
{
	texture_key = "static";
	hasExitTime = false;
	cycle = true;
	numFrames = 1;
	currFrame = 0;
	delay = 3;
}

AnimationData::AnimationData(std::string key, std::string path, int anim_numFrames)
{
	texture_key = key;
	hasExitTime = false;
	cycle = true;
	numFrames = anim_numFrames;
	currFrame = 0;
	delay = 3;

	UpdateTexMeshCache(key, path);
}

AnimationData::AnimationData(std::string key, std::string path, int anim_numFrames, int anim_delay, bool anim_hasExitTime, bool anim_cycle)
{
	assert(anim_numFrames > 0);

	texture_key = key;
	hasExitTime = anim_hasExitTime;
	cycle = anim_cycle;
	numFrames = anim_numFrames;
	currFrame = 0;
	delay = anim_delay;

	UpdateTexMeshCache(key, path);
}

void AnimationsComponent::AddAnimation(AnimationType type, AnimationData anim)
{
	anims[type] = anim;
}


// key should follow the convention: {entityname}_{animation}
// ie. an idle animation for egg would have key: egg_idle 
void AnimationData::UpdateTexMeshCache(std::string key, std::string path)
{
	ShadedMesh& resource = cache_resource(key);
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSpriteSheet(resource, numFrames, path, "animated_sprite");
		// careful, calling createSpriteSheet() instead of createSprite() binds
		// ShadedMesh's Texture component
		// to a 2D Array Texture instead of a 2D Texture
	}
}

// temporary animation system location
void AnimationSystem::ChangeAnimation(ECS::Entity e, AnimationType newAnim)
{
	if (e.has<AnimationsComponent>())
	{
		auto& anims = e.get<AnimationsComponent>();
		
		// do-nothing case
		if (anims.currentAnim == newAnim)
		{
			return;
		}

		// TODO check that the new anim exists

		// else update current animation and its data
		anims.currentAnim = newAnim;
		anims.currAnimData = anims.anims.at(anims.currentAnim);

		// TODO: add check for exit time

		// replace the entity's shaded mesh ref with a new one using the correct texture...?
		ShadedMesh& resource = cache_resource(anims.currAnimData.texture_key);
		e.get<ShadedMeshRef>() = ShadedMeshRef(resource);
		// feels like so much memory is leaking here idk
	}
}

// this is called every frame
void AnimationSystem::step()
{
	// for each Animation component...
	for (auto& anims : ECS::registry<AnimationsComponent>.components)
	{
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

		// reset the timer
		currAnim.delay_timer = currAnim.delay;
		
		 //calculate it's current frame...
		if (!currAnim.cycle)
		{
			// if the animation doesn't cycle and it's done, then stay on it's last frame
			currAnim.currFrame = max(currAnim.numFrames, currAnim.currFrame + 1);
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
}