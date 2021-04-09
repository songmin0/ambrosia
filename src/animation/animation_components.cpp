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

AnimationsComponent::AnimationsComponent(AnimationType type, const std::shared_ptr<AnimationData>& anim)
{
	anims[type] = anim;
	currentAnim = type;
	currAnimData = anims[type];
	referenceToCache = &cacheResource(anim->texture_key);
}

void AnimationsComponent::addAnimation(AnimationType type, const std::shared_ptr<AnimationData>& anim)
{
	if (anims.size() == 0)
	{
		currAnimData = anim;
		currentAnim = type;
		referenceToCache = &cacheResource(anim->texture_key);
	}
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
	currAnimData->currFrame = 0;

	// point the mesh ref to the new texture
	referenceToCache = &cacheResource(currAnimData->texture_key);
}

AnimationType AnimationsComponent::getCurrAnim()
{
	return currentAnim;
}

float AnimationsComponent::getCurrAnimProgress()
{
	if (currAnimData && currAnimData->numFrames != 0)
	{
		return currAnimData->currFrame / (float) (currAnimData->numFrames - 1);
	}

	return 0.f;
}

void AnimationData::updateTexMeshCache(const std::string& key, const std::string& path)
{
	ShadedMesh& resource = cacheResource(key);
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createAnimatedSprite(resource, numFrames, path, "animated_sprite");
	}
}