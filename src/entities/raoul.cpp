#include "raoul.hpp"

#include "rendering/render.hpp"
#include "animation/animation_components.hpp"
#include "game/turn_system.hpp"

ECS::Entity Raoul::createRaoul(vec2 position, float colourShift)
{
	auto entity = ECS::Entity();

	std::string key = "raoul_static";
	ShadedMesh& resource = cacheResource(key);
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, spritePath("players/raoul/raoul_static.png"), "textured");
	}

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	// note ShadedMeshRefs will only be rendered if there is no AnimationComponent attached to the entity
	entity.emplace<ShadedMeshRef>(resource);

	// Setting initial motion values
	Motion& motion = entity.emplace<Motion>();
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ 1.f, 1.f });

	// hitbox scaling
	auto hitboxScale = vec2({ 0.6f, 0.9f });
	motion.boundingBox = motion.scale * hitboxScale * vec2({ resource.texture.size.x, resource.texture.size.y });


	// Animations
	auto idle_anim = new AnimationData(
			"raoul_idle", spritePath("players/raoul/idle/idle"), 62, 1, false, true
	);
	AnimationsComponent& anims = entity.emplace<AnimationsComponent>(AnimationType::IDLE, *idle_anim);

	auto move_anim = new AnimationData(
			"raoul_move", spritePath("players/raoul/move/move"), 32, 1, false, true
	);
	anims.addAnimation(AnimationType::MOVE, *move_anim);

	auto attack1 = new AnimationData(
			"raoul_attack1", spritePath("players/raoul/attack1/attack1"), 59, 1, true, false, vec2({0.03f, 0.f})
	);
	anims.addAnimation(AnimationType::ATTACK1, *attack1);

	auto attack2 = new AnimationData(
			"raoul_attack2", spritePath("players/raoul/attack2/attack2"), 64, 1, true, false, vec2({ 0.03f, 0.f })
	);
	anims.addAnimation(AnimationType::ATTACK2, *attack2);

	auto attack3 = new AnimationData(
		"raoul_attack3", spritePath("players/raoul/attack3/attack3"), 40, 1, true, false
	);
	anims.addAnimation(AnimationType::ATTACK3, *attack3);

	auto hit = new AnimationData(
			"raoul_hit", spritePath("players/raoul/hit/hit"), 49, 1, true, false
	);
	anims.addAnimation(AnimationType::HIT, *hit);

	auto defeat = new AnimationData(
			"raoul_defeat", spritePath("players/raoul/defeat/defeat"), 61, 1, true, false, vec2({ 0.16f, 0.055f })
	);
	anims.addAnimation(AnimationType::DEFEAT, *defeat);

	// start off idle
	anims.changeAnimation(AnimationType::IDLE);

	entity.emplace<Raoul>();

	//Add the turn component
	entity.emplace<TurnSystem::TurnComponent>();

	//Add the player component
	entity.emplace<PlayerComponent>();

	// Temporary colour shift
	entity.emplace<ColourShift>().colour = colourShift;

	return entity;
};
