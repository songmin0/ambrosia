// Header
#include "raoul.hpp"
#include "render.hpp"
#include "animation_components.hpp"
#include "TurnSystem.hpp"

ECS::Entity Raoul::CreateRaoul(vec2 position, float colourShift)
{
	auto entity = ECS::Entity();

	std::string key = "raoul_static";
	ShadedMesh& resource = cache_resource(key);
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, sprite_path("players/raoul/raoul_static.png"), "textured");
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
		"raoul_idle", sprite_path("players/raoul/idle/idle"), 62, 1, false, true
	);
	AnimationsComponent& anims = entity.emplace<AnimationsComponent>(AnimationType::IDLE, *idle_anim);

	auto move_anim = new AnimationData(
		"raoul_move", sprite_path("players/raoul/move/move"), 47, 1, false, true
	);
	anims.AddAnimation(AnimationType::MOVE, *move_anim);

	auto attack3 = new AnimationData(
		"raoul_attack3", sprite_path("players/raoul/attack3/attack3"), 66, 1, true, false
	);
	anims.AddAnimation(AnimationType::ATTACK3, *attack3);

	auto attack1 = new AnimationData(
		"raoul_attack1", sprite_path("players/raoul/attack1/attack1"), 99, 1, true, false
	);
	anims.AddAnimation(AnimationType::ATTACK1, *attack1);

	auto attack2 = new AnimationData(
		"raoul_attack2", sprite_path("players/raoul/attack2/attack2"), 99, 1, true, false
	);
	anims.AddAnimation(AnimationType::ATTACK2, *attack2);

	auto hit = new AnimationData(
		"raoul_hit", sprite_path("players/raoul/hit/hit"), 66, 1, true, false
	);
	anims.AddAnimation(AnimationType::HIT, *hit);

	auto defeat = new AnimationData(
		"raoul_defeat", sprite_path("players/raoul/defeat/defeat"), 82, 1, true, false
	);
	anims.AddAnimation(AnimationType::DEFEAT, *defeat);

	// start off idle
	anims.ChangeAnimation(AnimationType::IDLE);

	entity.emplace<Raoul>();

	//Add the turn component
	entity.emplace<TurnSystem::TurnComponent>();

	//Add the player component
	entity.emplace<PlayerComponent>();

	// Temporary colour shift
	entity.emplace<ColourShift>().colour = colourShift;

	return entity;
};
