#pragma once
#include "common.hpp"
#include "render_components.hpp"
#include "render.hpp"
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
	int delay_timer = delay;

	AnimationData();
	AnimationData(std::string key, std::string path, int anim_numFrames);
	AnimationData(std::string key, std::string path, int anim_numFrames, int anim_delay, bool anim_hasExitTime, bool anim_cycle);
	void UpdateTexMeshCache(std::string key, std::string path);
};

struct AnimationsComponent
{
	std::unordered_map<AnimationType, AnimationData> anims;
	AnimationType currentAnim;
	AnimationData currAnimData;
	ShadedMesh* reference_to_cache;

	AnimationsComponent(AnimationType type, AnimationData anim);
	void AddAnimation(AnimationType type, AnimationData anim);
	void ChangeAnimation(AnimationType newAnim);
};