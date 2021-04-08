#pragma once
#include "tiny_ecs.hpp"
#include "game/common.hpp"

namespace Taji
{
	static constexpr vec2 SCALE(0.97f, 0.97f);
	static constexpr vec2 HITBOX_SCALE(0.5f, 0.85f);

	void initialize(ECS::Entity entity);
}
