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
	delay_timer = delay;
}

AnimationData::AnimationData(const std::string& key, const std::string& path, int anim_numFrames)
{
	texture_key = key;
	hasExitTime = false;
	cycle = true;
	numFrames = anim_numFrames;
	currFrame = 0;
	delay = 3;
	delay_timer = delay;

	updateTexMeshCache(key, path);
}

AnimationData::AnimationData(const std::string& key, const std::string& path, int anim_numFrames, int anim_delay, bool anim_hasExitTime, bool anim_cycle)
{
	assert(anim_numFrames > 0);

	texture_key = key;
	hasExitTime = anim_hasExitTime;
	cycle = anim_cycle;
	numFrames = anim_numFrames;
	currFrame = 0;
	delay = anim_delay;
	delay_timer = delay;

	updateTexMeshCache(key, path);
}

AnimationsComponent::AnimationsComponent(AnimationType type, const AnimationData& anim)
{
	anims[type] = anim;
	currentAnim = type;
	currAnimData = anim;
	reference_to_cache = &cacheResource(anim.texture_key);
}

void AnimationsComponent::addAnimation(AnimationType type, const AnimationData& anim)
{
	anims[type] = anim;
}

void AnimationsComponent::changeAnimation(AnimationType newAnim)
{
	// do nothing if no change, or the new animation doesn't exist
	if (currentAnim == newAnim || anims.count(newAnim) == 0)
	{
		return;
	}

	// else update current animation and its data
	currentAnim = newAnim;
	currAnimData = anims.at(currentAnim);

	// TODO: add check for exit time
	//// we probably don't need exit time since it's turn-based but keep it for now in case

	// point the mesh ref to the new texture
	reference_to_cache = &cacheResource(currAnimData.texture_key);
}

void AnimationData::updateTexMeshCache(const std::string& key, const std::string& path)
{
	ShadedMesh& resource = cacheResource(key);
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createAnimatedSprite(resource, numFrames, path, "animated_sprite");
	}
}