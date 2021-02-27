#include "effects.hpp"
#include "animation/animation_components.hpp"

ECS::Entity MouseClickFX::createMouseClickFX()
{
	auto entity = ECS::Entity();

	auto& motion = ECS::registry<Motion>.emplace(entity);
	motion.position = vec2(0.f);
	motion.angle = 0.f;
	motion.velocity = vec2(0.f);
	motion.scale = vec2(1.f);
	motion.boundingBox = vec2(0.f);

	auto effect_anim = new AnimationData("fx_mouseclick", uiPath("mouseclick_fx/mouseclick_fx"), 12, 1, false, false);
	AnimationsComponent& anims = entity.emplace<AnimationsComponent>(AnimationType::EFFECT, *effect_anim);
	anims.currAnimData.currFrame = 11; // start with the animation finished

	entity.emplace<MouseClickFX>();

	return entity;
}

ECS::Entity ActiveSkillFX::createActiveSkillFX()
{
	auto entity = ECS::Entity();

	std::string key = "fx_activeskill";
	ShadedMesh& resource = cacheResource(key);
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, uiPath("skill_buttons/active_fx.png"), "dynamic_texture");
	}

	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<SkillButtonComponent>(true, false, false);

	auto& motion = ECS::registry<Motion>.emplace(entity);
	motion.position = vec2(100, 1024 - 80);
	motion.angle = 0.f;
	motion.velocity = vec2(0.f);
	motion.scale = vec2(1.f);
	motion.boundingBox = vec2(0.f);

	entity.emplace<ActiveSkillFX>();

	return entity;
}