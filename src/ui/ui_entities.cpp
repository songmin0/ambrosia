#include "ui_entities.hpp"
#include "rendering/render.hpp"

ECS::Entity HPBar::createHPBar(vec2 position)
{
	auto entity = ECS::Entity();

	ShadedMesh& resource = cacheResource("hp_bar");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, uiPath("hp_bar.png"), "hp_bar");
	}

	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<RenderableComponent>(RenderLayer::UI);

	auto& motion = ECS::registry<Motion>.emplace(entity);
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = vec2(0.f);
	motion.scale = vec2(0.7f);
	motion.boundingBox = vec2(0.f);

	ECS::registry<HPBar>.emplace(entity);

	return entity;
}

ECS::Entity ToolTip::createToolTip(PlayerType player, SkillType skillType, vec2 position)
{
	auto entity = ECS::Entity();

	std::string skillString = "skill1";

	switch (skillType) {
	case SkillType::SKILL1:
		skillString = "skill1";
		break;
	case SkillType::SKILL2:
		skillString = "skill2";
		break;
	case SkillType::SKILL3:
		skillString = "skill3";
		break;
	default:
		entity.emplace<MoveToolTipComponent>();
		break;
	}

	ShadedMesh& resource = cacheResource(skillString + "_tooltip");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createPlayerSpecificMesh(resource, uiPath("tooltips/" + skillString), "skill_button");
	}

	ECS::registry<ShadedMeshRef>.emplace(entity, resource);
	entity.emplace<UIComponent>();
	entity.emplace<RenderableComponent>(RenderLayer::UI_TOOLTIP);

	auto& motion = ECS::registry<Motion>.emplace(entity);
	motion.position = position + vec2(resource.texture.size.x / 2.f, -resource.texture.size.y) / 2.f;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ 1.f, 1.f });
	motion.boundingBox = vec2(0.f);

	entity.emplace<SkillInfoComponent>(player, skillType);
	entity.emplace<VisibilityComponent>().isVisible = false;

	entity.emplace<ToolTip>();

	return entity;
}

ECS::Entity ToolTip::createMoveToolTip(vec2 position)
{
	auto entity = ECS::Entity();
	entity.emplace<MoveToolTipComponent>();

	ShadedMesh& resource = cacheResource("move_tooltip");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, uiPath("tooltips/move_tooltip.png"), "textured");
	}

	ECS::registry<ShadedMeshRef>.emplace(entity, resource);
	entity.emplace<UIComponent>();
	entity.emplace<RenderableComponent>(RenderLayer::UI_TOOLTIP);

	auto& motion = ECS::registry<Motion>.emplace(entity);
	motion.position = position + vec2(resource.texture.size.x / 2.f, -resource.texture.size.y) / 2.f;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ 1.f, 1.f });
	motion.boundingBox = vec2(0.f);

	entity.emplace<SkillInfoComponent>(PlayerType::RAOUL, SkillType::MOVE);
	entity.emplace<VisibilityComponent>().isVisible = false;

	entity.emplace<ToolTip>();

	return entity;
}