#pragma once
#include "common.hpp"
#include "entities/tiny_ecs.hpp"

enum class MoveType
{
	TO_CLOSEST_PLAYER,
	TO_FARTHEST_PLAYER,
	TO_WEAKEST_PLAYER,
	TO_WEAKEST_MOB,
	AWAY_CLOSEST_PLAYER
};

struct MovementType
{
	MoveType moveType;
};