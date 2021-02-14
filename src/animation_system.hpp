#pragma once
#include "common.hpp"
#include "render_components.hpp"
#include "render.hpp"
#include <vector>
#include <unordered_map> 

class AnimationSystem
{
public:
	void step();
	void checkAnimation(ECS::Entity& entity);
};
