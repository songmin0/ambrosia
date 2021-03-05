#include "map_objects.hpp"
#include "rendering/render.hpp"

ECS::Entity CheeseBlob::createCheeseBlob(vec2 position)
{
	auto entity = ECS::Entity();

	std::string key = "cheeseblob_static";
	ShadedMesh& resource = cacheResource(key);
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, objectsPath("cheese-texture.png"), "bloblike");
	}

	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<RenderableComponent>(RenderLayer::MAP_OBJECT);

	// Setting initial motion values
	Motion& motion = entity.emplace<Motion>();
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ 1.f, 1.f });
	
	entity.emplace<CheeseBlob>();

	return entity;
};

ECS::Entity DessertForeground::createDessertForeground(vec2 position)
{
	auto entity = ECS::Entity();

	std::string key = "dessertmap_foreground";
	ShadedMesh& resource = cacheResource(key);
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, mapsPath("dessert-arena/dessert-arena-front.png"), "textured");
	}

	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<RenderableComponent>(RenderLayer::MAP_FOREGROUND);

	// Setting initial motion values
	Motion& motion = entity.emplace<Motion>();
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ 1.f, 1.f });

	entity.emplace<DessertForeground>();

	return entity;
};
