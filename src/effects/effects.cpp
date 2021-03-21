#include "effects.hpp"
#include "animation/animation_components.hpp"

ECS::Entity MouseClickFX::createMouseClickFX()
{
	// There should only ever be one of this type of entity
	while (!ECS::ComponentContainer<MouseClickFX>().entities.empty())
	{
		ECS::ContainerInterface::removeAllComponentsOf(ECS::registry<MouseClickFX>.entities.back());
	}

	auto entity = ECS::Entity();

	ShadedMesh& resource = cacheResource("mouseclick_static");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, uiPath("mouseclick_fx/mouseclick_fx_005.png"), "textured");
	}

	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<UIComponent>();
	entity.emplace<RenderableComponent>(RenderLayer::CLICK_FX);

	auto& motion = ECS::registry<Motion>.emplace(entity);
	motion.position = vec2(0.f);
	motion.angle = 0.f;
	motion.velocity = vec2(0.f);
	motion.scale = vec2(1.f);
	motion.boundingBox = vec2(0.f);

	auto effect_anim = AnimationData("fx_mouseclick", uiPath("mouseclick_fx/mouseclick_fx"), 12, 1, false, false);
	AnimationsComponent& anims = entity.emplace<AnimationsComponent>(AnimationType::EFFECT, std::make_shared<AnimationData>(effect_anim));
	anims.currAnimData->currFrame = 11; // start with the animation finished

	entity.emplace<MouseClickFX>();

	return entity;
}

ECS::Entity ActiveSkillFX::createActiveSkillFX()
{
	// There should only ever be one of this type of entity
	while (!ECS::ComponentContainer<ActiveSkillFX>().entities.empty())
	{
		ECS::ContainerInterface::removeAllComponentsOf(ECS::registry<ActiveSkillFX>.entities.back());
	}

	auto entity = ECS::Entity();

	std::string key = "fx_activeskill";
	ShadedMesh& resource = cacheResource(key);
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, uiPath("skill_buttons/active_fx.png"), "dynamic_texture");
	}

	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<UIComponent>();
	entity.emplace<RenderableComponent>(RenderLayer::UI_ACTIVE_SKILL_FX);
	entity.emplace<ButtonStateComponent>(true, false);
	entity.emplace<VisibilityComponent>().isVisible = false;

	auto& motion = ECS::registry<Motion>.emplace(entity);
	motion.position = vec2(100, 1024 - 80);
	motion.angle = 0.f;
	motion.velocity = vec2(0.f);
	motion.scale = vec2(1.f);
	motion.boundingBox = vec2(0.f);

	entity.emplace<ActiveSkillFX>();

	return entity;
}

ECS::Entity commonInitFX(const std::string& key, const int numFrames, const bool doesCycle)
{
	auto entity = ECS::Entity();

	ShadedMesh& resource = cacheResource(key);
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, fxPath(key + "/" + key + "_000.png"), "textured");
	}
	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<RenderableComponent>(RenderLayer::SKILL);
	entity.emplace<SkillFX>();

	auto effect_anim = AnimationData(key + "_anim", fxPath(key + "/" + key), numFrames, 1, false, doesCycle);
	AnimationsComponent& anims = entity.emplace<AnimationsComponent>(AnimationType::EFFECT, std::make_shared<AnimationData>(effect_anim));

	return entity;
};

ECS::Entity BuffedFX::createBuffedFX(vec2 position, vec2 scale)
{
	auto entity = commonInitFX("buffed", 30, true);

	auto& motion = ECS::registry<Motion>.emplace(entity);
	motion.position = position;
	motion.scale = scale;

	return entity;
}

ECS::Entity DebuffedFX::createDebuffedFX(vec2 position, vec2 scale)
{
	auto entity = commonInitFX("debuffed", 31, true);

	auto& motion = ECS::registry<Motion>.emplace(entity);
	motion.position = position;
	motion.scale = scale;

	return entity;
}

ECS::Entity HealedFX::createHealedFX(vec2 position, vec2 scale)
{
	auto entity = commonInitFX("healed", 56, false);

	auto& motion = ECS::registry<Motion>.emplace(entity);
	motion.position = position;
	motion.scale = scale;

	return entity;
}

ECS::Entity ShieldedFX::createShieldedFX(vec2 position, vec2 scale)
{
	auto entity = commonInitFX("shielded", 30, true);

	auto& motion = ECS::registry<Motion>.emplace(entity);
	motion.position = position;
	motion.scale = scale;

	return entity;
}

ECS::Entity Candy1FX::createCandy1FX(vec2 position, vec2 scale)
{
	auto entity = commonInitFX("candy1", 18, false);

	auto& motion = ECS::registry<Motion>.emplace(entity);
	motion.position = position;
	motion.scale = scale;

	return entity;
}

ECS::Entity Candy2FX::createCandy2FX(vec2 position, vec2 scale)
{
	auto entity = commonInitFX("candy2", 18, false);

	auto& motion = ECS::registry<Motion>.emplace(entity);
	motion.position = position;
	motion.scale = scale;

	return entity;
}

ECS::Entity BlueberriedFX::createBlueberriedFX(vec2 position, vec2 scale)
{
	auto entity = commonInitFX("blueberried", 11, false);

	auto& motion = ECS::registry<Motion>.emplace(entity);
	motion.position = position;
	motion.scale = scale;

	return entity;
}

ECS::Entity StunnedFX::createStunnedFX(vec2 position, vec2 scale)
{
	auto entity = commonInitFX("stunned", 11, true);

	auto& motion = ECS::registry<Motion>.emplace(entity);
	motion.position = position;
	motion.scale = scale;

	return entity;
}