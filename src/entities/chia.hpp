#pragma once
#include "tiny_ecs.hpp"
#include "game/common.hpp"

namespace Chia
{
	static constexpr vec2 SCALE(1.f);
	static constexpr vec2 HITBOX_SCALE(0.5f, 0.8f);

	void initialize(ECS::Entity entity);
}
