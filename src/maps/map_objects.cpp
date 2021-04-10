#include "map_objects.hpp"

ECS::Entity CheeseBlob::createCheeseBlob(vec2 position)
{
	auto entity = ECS::Entity();

	ShadedMesh& resource = cacheResource("cheeseblob_static");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, objectsPath("cheese-texture.png"), "distendable");
	}
	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<DistendableComponent>();
	entity.emplace<RenderableComponent>(RenderLayer::MAP_OBJECT);
	entity.emplace<Motion>().position = position;

	entity.emplace<CheeseBlob>();
	return entity;
};

ECS::Entity DessertForeground::createDessertForeground(vec2 position)
{
	// There should only ever be one of this type of entity
	while (!ECS::registry<DessertForeground>.entities.empty()) {
		ECS::ContainerInterface::removeAllComponentsOf(ECS::registry<DessertForeground>.entities.back());
	}

	auto entity = ECS::Entity();

	ShadedMesh& resource = cacheResource("dessertmap_foreground");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, mapsPath("dessert-arena/dessert-arena-front.png"), "textured");
	}
	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<RenderableComponent>(RenderLayer::MAP_FOREGROUND);
	entity.emplace<Motion>().position = position;
	entity.emplace<ParallaxComponent>(vec2(1.3f, 1.15f));

	entity.emplace<DessertForeground>();
	return entity;
};

ECS::Entity DessertBackground::createDessertBackground(vec2 position)
{
	// There should only ever be one of this type of entity
	while (!ECS::registry<DessertBackground>.entities.empty()) {
		ECS::ContainerInterface::removeAllComponentsOf(ECS::registry<DessertBackground>.entities.back());
	}

	auto entity = ECS::Entity();

	ShadedMesh& resource = cacheResource("dessertmap_background");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, mapsPath("dessert-arena/dessert-arena-back.png"), "textured");
	}
	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<RenderableComponent>(RenderLayer::MAP_BACKGROUND);
	entity.emplace<Motion>().position = position;
	entity.emplace<ParallaxComponent>(vec2(0.7f, 0.85f));

	entity.emplace<DessertBackground>();
	return entity;
};

ECS::Entity BBQBackground::createBBQBackground(vec2 position)
{
	// There should only ever be one of this type of entity
	while (!ECS::registry<BBQBackground>.entities.empty()) {
		ECS::ContainerInterface::removeAllComponentsOf(ECS::registry<BBQBackground>.entities.back());
	}

	auto entity = ECS::Entity();
	ShadedMesh& resource = cacheResource("bbq_background");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, mapsPath("bbq/bbq-back.png"), "textured");
	}
	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<RenderableComponent>(RenderLayer::MAP_BACKGROUND);
	entity.emplace<Motion>().position = position;
	entity.emplace<ParallaxComponent>(vec2(0.6f, 0.6f));

	entity.emplace<BBQBackground>();
	return entity;
};

ECS::Entity BBQFire::createBBQFire(vec2 position, RenderLayer layer, vec2 scale)
{
	auto entity = ECS::Entity();
	ShadedMesh& resource = cacheResource("bbq_fire");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, mapsPath("bbq/fire.png"), "fire");
	}
	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<RenderableComponent>(layer);
	auto& motion = entity.emplace<Motion>();
	motion.position = position;
	motion.scale = scale;

	entity.emplace<BBQFire>();
	return entity;
};
