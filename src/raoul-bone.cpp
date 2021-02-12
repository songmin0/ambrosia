// Header
#include "raoul-bone.hpp"
#include "render.hpp"
#include "animation_components.hpp"
#include "TurnSystem.hpp"

ECS::Entity RaoulBone::CreateRaoulBone(vec2 position)
{
	auto entity = ECS::Entity();

	std::string key = "raoul_bone";
	ShadedMesh& resource = cache_resource(key);
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, sprite_path("players/raoul/raoul-bone.png"), "textured");
	}

	entity.emplace<ShadedMeshRef>(resource);

	// Setting initial motion values
	Motion& motion = entity.emplace<Motion>();
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ 0.5f, 0.5f });
	motion.boundingBox = motion.scale * vec2({ resource.texture.size.x, resource.texture.size.y });

	entity.emplace<RaoulBone>();

	return entity;
};
