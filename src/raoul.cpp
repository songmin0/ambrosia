// Header
#include "raoul.hpp"
#include "render.hpp"
#include "animation_components.hpp"

ECS::Entity Raoul::CreateRaoul(vec2 position)
{
	auto entity = ECS::Entity();

	std::string key = "raoul_static";
	ShadedMesh& resource = cache_resource(key);
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, sprite_path("players/raoul/raoul_static.png"), "textured");
	}

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	entity.emplace<ShadedMeshRef>(resource);

	// Setting initial motion values
	Motion& motion = entity.emplace<Motion>();
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ 1.f, 1.f }) * static_cast<vec2>(resource.texture.size);

	// Animations
	AnimationsComponent& anims = entity.emplace<AnimationsComponent>();
	auto idle_anim = new AnimationData(
		"raoul_idle", sprite_path("players/raoul/idle/idle"), 62, 1, false, true
	);

	anims.AddAnimation(AnimationType::IDLE, *idle_anim);

	// test animation hook-up
	AnimationSystem::ChangeAnimation(entity, AnimationType::IDLE);

	entity.emplace<Raoul>();

	return entity;
};
