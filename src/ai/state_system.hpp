#pragma once

#include "game/common.hpp"
#include "entities/tiny_ecs.cpp"

#include "behaviour_tree.hpp"

class StateSystem 
{
public:
	StateSystem() = default;
	~StateSystem() = default;

	void step(float elapsed_ms);
};