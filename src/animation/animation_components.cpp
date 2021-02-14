#pragma once
#include "animation_components.hpp"

AnimationData::AnimationData()
{
	texture_key = "static";
	hasExitTime = false;
	isCycle = true;
	numFrames = 1;
	currFrame = 0;
	delay = 3;
	delayTimer = delay;
	offset = vec2(0.f);
}

AnimationData::AnimationData(const std::string& key, const std::string& path, int animNumFrames, int animDelay, bool animHasExitTime, bool animIsCycle, vec2 animOffset)
{
	assert(animNumFrames > 0);

	texture_key = key;
	hasExitTime = animHasExitTime;
	isCycle = animIsCycle;
	numFrames = animNumFrames;
	currFrame = 0;
	delay = animDelay;
	delayTimer = delay;
	offset = animOffset;

	updateTexMeshCache(key, path);
}

AnimationsComponent::AnimationsComponent(AnimationType type, const AnimationData& anim)
{
	anims[type] = anim;
	currentAnim = type;
	currAnimData = anim;
	referenceToCache = &cacheResource(anim.texture_key);
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
	referenceToCache = &cacheResource(currAnimData.texture_key);
}

void AnimationData::updateTexMeshCache(const std::string& key, const std::string& path)
{
	ShadedMesh& resource = cacheResource(key);
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createAnimatedSprite(resource, numFrames, path, "animated_sprite");
	}
}