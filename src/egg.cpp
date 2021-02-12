// Header
#include "egg.hpp"
#include "render.hpp"
#include "animation_components.hpp"
#include "ai.hpp"
#include "TurnSystem.hpp"

ECS::Entity Egg::CreateEgg(vec2 position)
{
	auto entity = ECS::Entity();

	std::string key = "egg_static";
	ShadedMesh& resource = cache_resource(key);
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, sprite_path("enemies/egg/egg_static.png"), "textured");
	}

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	// note ShadedMeshRefs will only be rendered if there is no AnimationComponent attached to the entity
	entity.emplace<ShadedMeshRef>(resource);

	// Give it a Mob component
	entity.emplace<AISystem::MobComponent>();

	entity.emplace<Egg>();

	entity.emplace<TurnSystem::TurnComponent>();

	// Setting initial motion values
	Motion& motion = entity.emplace<Motion>();
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ 0.8f, 0.8f });


	// hitbox scaling
	auto hitboxScale = vec2({ 0.7f, 0.8f });
	motion.boundingBox = motion.scale * hitboxScale * vec2({ resource.texture.size.x, resource.texture.size.y });

	// Animations
	auto idle_anim = new AnimationData(
		"egg_idle", sprite_path("enemies/egg/idle/idle"), 76, 1, false, true
	);
	 AnimationsComponent& anims = entity.emplace<AnimationsComponent>(AnimationType::IDLE, *idle_anim);

	auto move_anim = new AnimationData(
		"egg_move", sprite_path("enemies/egg/move/move"), 51, 1, false, true
	);
	anims.AddAnimation(AnimationType::MOVE, *move_anim);

	// start off moving
	anims.ChangeAnimation(AnimationType::MOVE);

	return entity;
};
