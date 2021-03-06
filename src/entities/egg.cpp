// Header
#include "egg.hpp"

#include "rendering/render.hpp"
#include "animation/animation_components.hpp"
#include "ai/ai.hpp"
#include "ai/behaviour_tree.hpp"
#include "game/turn_system.hpp"
#include "ui/ui_entities.hpp"

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
	entity.emplace<RenderableComponent>(RenderLayer::PLAYER_AND_MOB);

	// Give it a Mob component
	entity.emplace<AISystem::MobComponent>();
	// Set appropriate Behaviour Tree
	//entity.emplace<EggBehaviourTree>();
	//EggBehaviourTree bt;

	entity.emplace<Egg>();

	auto& btType = entity.emplace<BehaviourTreeType>();
	btType.mobType = MobType::EGG;

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
	auto hitboxScale = vec2({ 0.65f, 0.7f });
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

	//Add HP bar
	statsComponent.healthBar = HPBar::createHPBar({ motion.position.x, motion.position.y - 150.0f });
	ECS::registry<HPBar>.get(statsComponent.healthBar).offset = { 0.0f,-150.0f };
	ECS::registry<HPBar>.get(statsComponent.healthBar).statsCompEntity = entity;


	// Initialize skills
	auto& skillComponent = entity.emplace<SkillComponent>();

	// Egg shell projectile
	SkillParams eggShellParams;
	eggShellParams.instigator = entity;
	eggShellParams.animationType = AnimationType::ATTACK1;
	eggShellParams.delay = 0.6f;
	eggShellParams.damage = 10.f;
	eggShellParams.collidesWith = CollisionGroup::PLAYER;
	eggShellParams.soundEffect = SoundEffect::PROJECTILE;
	skillComponent.addSkill(SkillType::SKILL1, std::make_shared<ProjectileSkill>(eggShellParams, ProjectileType::EGG_SHELL));

	return entity;
};
