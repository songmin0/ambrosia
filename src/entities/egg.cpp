// Header
#include "egg.hpp"

#include "rendering/render.hpp"
#include "animation/animation_components.hpp"
#include "ai/ai.hpp"
#include "game/turn_system.hpp"

ECS::Entity Egg::createEgg(vec2 pos)
{
	auto entity = ECS::Entity();

	std::string key = "egg_static";
	ShadedMesh& resource = cacheResource(key);
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, spritePath("enemies/egg/egg_static.png"), "textured");
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
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ 0.8f, 0.8f });
	motion.orientation = -1;
	motion.colliderType = CollisionGroup::MOB;
	motion.collidesWith = CollisionGroup::PLAYER;


	// hitbox scaling
	auto hitboxScale = vec2({ 0.7f, 0.8f });
	motion.boundingBox = motion.scale * hitboxScale * vec2({ resource.texture.size.x, resource.texture.size.y });

	// Animations
	auto idle_anim = new AnimationData(
			"egg_idle", spritePath("enemies/egg/idle/idle"), 76, 1, false, true
	);
	AnimationsComponent& anims = entity.emplace<AnimationsComponent>(AnimationType::IDLE, *idle_anim);

	auto move_anim = new AnimationData(
			"egg_move", spritePath("enemies/egg/move/move"), 51, 1, false, true
	);
	anims.addAnimation(AnimationType::MOVE, *move_anim);

	auto hit_anim = new AnimationData(
			"egg_hit", spritePath("enemies/egg/hit/hit"), 29, 1, true, false
	);
	anims.addAnimation(AnimationType::HIT, *hit_anim);

	auto attack1_anim = new AnimationData(
			"egg_attack1", spritePath("enemies/egg/attack1/attack1"), 36, 1, true, false
	);
	anims.addAnimation(AnimationType::ATTACK1, *attack1_anim);

	auto defeat_anim = new AnimationData(
			"egg_defeat", spritePath("enemies/egg/defeat/defeat"), 48, 1, true, false
	);
	anims.addAnimation(AnimationType::DEFEAT, *defeat_anim);

	// start off moving
	anims.changeAnimation(AnimationType::MOVE);

	// Initialize stats
	auto& statsComponent = entity.emplace<StatsComponent>();
	statsComponent.stats[StatType::HP] = 100.f;
	statsComponent.stats[StatType::AMBROSIA] = 0.f;
	statsComponent.stats[StatType::STRENGTH] = 1.f;

	return entity;
};
