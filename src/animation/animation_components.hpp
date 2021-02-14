#pragma once
#include "game/common.hpp"
#include "rendering/render_components.hpp"
#include "rendering/render.hpp"

#include <vector>
#include <unordered_map> 

enum class AnimationType { STATIC, IDLE, MOVE, ATTACK1, ATTACK2, ATTACK3, HIT, DEFEAT };

struct AnimationData
{
	std::string texture_key;
	bool hasExitTime;
	bool cycle;
	int numFrames;
	int currFrame;
	int delay;
	int delay_timer;

	AnimationData();
	AnimationData(const std::string& key, const std::string& path, int anim_numFrames);
	AnimationData(const std::string& key, const std::string& path, int anim_numFrames, int anim_delay, bool anim_hasExitTime, bool anim_cycle);
	void updateTexMeshCache(const std::string& key, const std::string& path);
};

struct AnimationsComponent
{
	std::unordered_map<AnimationType, AnimationData> anims;
	AnimationType currentAnim;
	AnimationData currAnimData;
	ShadedMesh* reference_to_cache;

	AnimationsComponent(AnimationType type, const AnimationData& anim);
	void addAnimation(AnimationType type, const AnimationData& anim);
	void changeAnimation(AnimationType newAnim);
};