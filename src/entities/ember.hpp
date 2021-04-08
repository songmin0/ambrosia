#pragma once
#include "tiny_ecs.hpp"
#include "game/common.hpp"

namespace Ember
{
	static constexpr vec2 SCALE(1.f);
	static constexpr vec2 HITBOX_SCALE(0.4f, 0.6f);

	void initialize(ECS::Entity entity);
}
