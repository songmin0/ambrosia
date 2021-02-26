#pragma once
#include "game/common.hpp"
#include "rendering/render_components.hpp"
#include "rendering/render.hpp"

#include <vector>
#include <unordered_map> 

enum class AnimationType { STATIC, IDLE, MOVE, ATTACK1, ATTACK2, ATTACK3, HIT, DEFEAT, ACTIVE, INACTIVE, DISABLED, EFFECT };

struct AnimationData
{
	std::string texture_key;
	bool hasExitTime;
	bool isCycle;
	int numFrames;
	int currFrame;
	int delay;
	int delayTimer;
	vec2 offset;

	AnimationData();
	AnimationData(const std::string& key, const std::string& path, int animNumFrames, int animDelay = 1, bool animHasExitTime = false, bool animIsCycle = true, vec2 animOffset = vec2(0.f));
	void updateTexMeshCache(const std::string& key, const std::string& path);
};

struct AnimationsComponent
{
	std::unordered_map<AnimationType, AnimationData> anims;
	AnimationType currentAnim;
	AnimationData currAnimData;
	ShadedMesh* referenceToCache;

	AnimationsComponent(AnimationType type, const AnimationData& anim);
	void addAnimation(AnimationType type, const AnimationData& anim);
	void changeAnimation(AnimationType newAnim);
};