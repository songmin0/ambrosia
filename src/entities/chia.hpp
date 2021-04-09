#pragma once
#include "tiny_ecs.hpp"
#include "game/common.hpp"
#include "skills/skill_component.hpp"

struct Chia
{
public:
	static ECS::Entity createChia(json initialStats);
	static ECS::Entity createChia(vec2 position);
private:
	static ECS::Entity commonInit();
	static void addSkill1(ECS::Entity entity, SkillComponent& skillComponent);
	static void addSkill2(ECS::Entity entity, SkillComponent& skillComponent);
	static void addSkill3(ECS::Entity entity, SkillComponent& skillComponent);
};
