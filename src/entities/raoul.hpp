#pragma once
#include "tiny_ecs.hpp"
#include "game/common.hpp"
#include "skills/skill_component.hpp"

namespace Raoul
{
	static constexpr vec2 SCALE(1.f);
	static constexpr vec2 HITBOX_SCALE(0.6f, 0.9f);

	void initialize(ECS::Entity entity);

	void addSkill1(ECS::Entity entity, SkillComponent& skillComponent);
	void addSkill2(ECS::Entity entity, SkillComponent& skillComponent);
	void addSkill3(ECS::Entity entity, SkillComponent& skillComponent);
}
