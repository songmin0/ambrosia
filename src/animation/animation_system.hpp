#pragma once
#include "game/common.hpp"
#include "rendering/render_components.hpp"
#include "rendering/render.hpp"

#include <vector>
#include <unordered_map> 

class AnimationSystem
{
public:
	void step();
	void checkAnimation(ECS::Entity& entity);
};
