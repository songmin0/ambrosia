#include "map_objects.hpp"
#include "rendering/render.hpp"

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
	auto entity = ECS::Entity();

	ShadedMesh& resource = cacheResource("dessertmap_foreground");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, mapsPath("dessert-arena/dessert-arena-front.png"), "textured");
	}
	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<RenderableComponent>(RenderLayer::MAP_FOREGROUND);
	entity.emplace<Motion>().position = position;

	entity.emplace<DessertForeground>();
	return entity;
};
