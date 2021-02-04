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

AnimationsComponent::AnimationsComponent(AnimationType type, AnimationData anim)
{
	anims[type] = anim;
	currentAnim == type;
	currAnimData = anim;
	reference_to_cache = &cache_resource(anim.texture_key);
}

void AnimationsComponent::AddAnimation(AnimationType type, AnimationData anim)
{
	anims[type] = anim;
}

// temporary animation system location
void AnimationsComponent::ChangeAnimation(AnimationType newAnim)
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
	//// it's probably better to check for exit time elsewhere...?

	// replace the mesh ref with the new texture
	reference_to_cache = &cache_resource(currAnimData.texture_key);
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