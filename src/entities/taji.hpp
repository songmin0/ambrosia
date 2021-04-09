#pragma once
#include "tiny_ecs.hpp"
#include "game/common.hpp"
#include "skills/skill_component.hpp"

struct Taji
{
public:
	static ECS::Entity createTaji(json initialStats);
	static ECS::Entity createTaji(vec2 pos);
private:
	static ECS::Entity commonInit();
	static void addSkill1(ECS::Entity entity, SkillComponent& skillComponent);
	static void addSkill2(ECS::Entity entity, SkillComponent& skillComponent);
	static void addSkill3(ECS::Entity entity, SkillComponent& skillComponent);
};
