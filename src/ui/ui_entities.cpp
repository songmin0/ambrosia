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

	auto& motion = ECS::registry<Motion>.emplace(entity);
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = vec2(0.f);
	motion.scale = vec2(0.7f);
	motion.boundingBox = vec2(0.f);

	ECS::registry<HPBar>.emplace(entity);

	return entity;
}