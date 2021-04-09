#pragma once
#include "tiny_ecs.hpp"
#include "game/common.hpp"

namespace Player
{
	ECS::Entity create(PlayerType type, const json& configValues);
	void prepareForNextMap(ECS::Entity entity, const json& configValues);

	void enableRendering(ECS::Entity entity);
	void disableRendering(ECS::Entity entity);
}
