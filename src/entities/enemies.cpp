// Header
#include "enemies.hpp"

#include "rendering/render.hpp"
#include "animation/animation_components.hpp"
#include "ai/ai.hpp"
#include "ai/behaviour_tree.hpp"
#include "game/turn_system.hpp"
#include "ui/ui_entities.hpp"

ECS::Entity Pepper::createPepper(vec2 pos)
{
	auto entity = ECS::Entity();

	ShadedMesh& resource = cacheResource("pepper_static");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, spritePath("enemies/pepper/pepper_static.png"), "textured");
	}
	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<RenderableComponent>(RenderLayer::PLAYER_AND_MOB);

	// TODO: Figure this out for Pepper
	// Give it a Mob component
	entity.emplace<AISystem::MobComponent>();
	auto& btType = entity.emplace<BehaviourTreeType>();
	btType.mobType = MobType::EGG;

	entity.emplace<TurnSystem::TurnComponent>();

	// Setting initial motion values
	Motion& motion = entity.emplace<Motion>();
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = vec2(0.f);
	motion.scale = vec2(0.9f);
	motion.orientation = -1;
	auto hitboxScale = vec2({ 0.4f, 0.7f });
	motion.boundingBox = motion.scale * hitboxScale * vec2({ resource.texture.size.x, resource.texture.size.y });
	motion.colliderType = CollisionGroup::MOB;
	motion.collidesWith = CollisionGroup::PLAYER;

	// Animations
	auto idle_and_run = new AnimationData(
			"pepper_idle", spritePath("enemies/pepper/idle/idle"), 74);
	AnimationsComponent& anims = entity.emplace<AnimationsComponent>(AnimationType::IDLE, *idle_and_run);
	anims.addAnimation(AnimationType::MOVE, *idle_and_run);

	auto hit_anim = new AnimationData(
			"pepper_hit", spritePath("enemies/pepper/hit/hit"), 24, 1, true, false
	);
	anims.addAnimation(AnimationType::HIT, *hit_anim);

	auto attack1_anim = new AnimationData(
			"pepper_attack1", spritePath("enemies/pepper/attack1/attack1"), 45, 1, true, false
	);
	anims.addAnimation(AnimationType::ATTACK1, *attack1_anim);

	auto defeat_anim = new AnimationData(
			"pepper_defeat", spritePath("enemies/pepper/defeat/defeat"), 41, 1, true, false
	);
	anims.addAnimation(AnimationType::DEFEAT, *defeat_anim);

	// Initialize stats
	auto& statsComponent = entity.emplace<StatsComponent>();
	statsComponent.stats[StatType::HP] = 100.f;
	statsComponent.stats[StatType::AMBROSIA] = 0.f;
	statsComponent.stats[StatType::STRENGTH] = 1.f;

	//Add HP bar
	statsComponent.healthBar = HPBar::createHPBar({ motion.position.x, motion.position.y - 150.0f });
	ECS::registry<HPBar>.get(statsComponent.healthBar).offset = { 0.0f, -240.0f };
	ECS::registry<HPBar>.get(statsComponent.healthBar).statsCompEntity = entity;

	// Initialize skills
	auto& skillComponent = entity.emplace<SkillComponent>();

	// Melee hit
	SkillParams meleeParams;
	meleeParams.instigator = entity;
	meleeParams.animationType = AnimationType::ATTACK1;
	meleeParams.delay = 1.f;
	meleeParams.damage = 40.f;
	meleeParams.range = 200.f;
	meleeParams.collideWithMultipleEntities = false;
	meleeParams.collidesWith = CollisionGroup::PLAYER;
	meleeParams.soundEffect = SoundEffect::MELEE;
	skillComponent.addSkill(SkillType::SKILL1, std::make_shared<MeleeSkill>(meleeParams));

	entity.emplace<Pepper>();
	return entity;
};


ECS::Entity Milk::createMilk(vec2 pos, float orientation)
{
	auto entity = ECS::Entity();

	ShadedMesh& resource = cacheResource("milk_static");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, spritePath("enemies/milk/idle/idle_000.png"), "textured");
	}
	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<RenderableComponent>(RenderLayer::PLAYER_AND_MOB);

	// TODO: Figure this out
	entity.emplace<AISystem::MobComponent>();
	auto& btType = entity.emplace<BehaviourTreeType>();
	btType.mobType = MobType::EGG;

	entity.emplace<TurnSystem::TurnComponent>();

	Motion& motion = entity.emplace<Motion>();
	motion.position = pos;
	motion.orientation = orientation;
	motion.scale = vec2(1.f);
	auto hitboxScale = vec2({ 0.4f, 0.7f });
	motion.boundingBox = motion.scale * hitboxScale * vec2({ resource.texture.size.x, resource.texture.size.y });
	motion.colliderType = CollisionGroup::MOB;
	motion.collidesWith = CollisionGroup::PLAYER;

	// Animations
	auto idle = new AnimationData("milk_idle", spritePath("enemies/milk/idle/idle"), 30);
	AnimationsComponent& anims = entity.emplace<AnimationsComponent>(AnimationType::IDLE, *idle);
	
	auto move = new AnimationData("milk_move", spritePath("enemies/milk/move/move"), 20);
	anims.addAnimation(AnimationType::MOVE, *move);

	auto hit_anim = new AnimationData("milk_hit", spritePath("enemies/milk/hit/hit"), 12, 1, true, false);
	anims.addAnimation(AnimationType::HIT, *hit_anim);

	auto attack1_anim = new AnimationData("milk_attack1", spritePath("enemies/milk/attack1/attack1"), 27, 1, true, false);
	anims.addAnimation(AnimationType::ATTACK1, *attack1_anim);

	auto defeat_anim = new AnimationData("milk_defeat", spritePath("enemies/milk/defeat/defeat"), 23, 1, true, false, vec2({ 0.15f, 0.f }));
	anims.addAnimation(AnimationType::DEFEAT, *defeat_anim);

	// Initialize stats
	auto& statsComponent = entity.emplace<StatsComponent>();
	statsComponent.stats[StatType::HP] = 100.f;
	statsComponent.stats[StatType::AMBROSIA] = 0.f;
	statsComponent.stats[StatType::STRENGTH] = 1.f;

	//Add HP bar
	statsComponent.healthBar = HPBar::createHPBar({ motion.position.x, motion.position.y - 150.0f });
	ECS::registry<HPBar>.get(statsComponent.healthBar).offset = { 0.0f, -240.0f };
	ECS::registry<HPBar>.get(statsComponent.healthBar).statsCompEntity = entity;

	// Initialize skills
	auto& skillComponent = entity.emplace<SkillComponent>();

	// Heal
	SkillParams rangedAttackParams;
	rangedAttackParams.instigator = entity;
	rangedAttackParams.animationType = AnimationType::ATTACK1;
	rangedAttackParams.delay = 0.3f;
	rangedAttackParams.damage = 20.f; // this is "healing"
	rangedAttackParams.collidesWith = CollisionGroup::MOB;
	rangedAttackParams.soundEffect = SoundEffect::PROJECTILE;
	skillComponent.addSkill(SkillType::SKILL1, std::make_shared<ProjectileSkill>(rangedAttackParams, ProjectileType::HEAL_ORB));

	// Use a ranged attack if there's no ally to heal
	SkillParams rangedAttackParams;
	rangedAttackParams.instigator = entity;
	rangedAttackParams.animationType = AnimationType::ATTACK1;
	rangedAttackParams.delay = 0.3f;
	rangedAttackParams.damage = 15.f;
	rangedAttackParams.collidesWith = CollisionGroup::PLAYER;
	rangedAttackParams.soundEffect = SoundEffect::PROJECTILE;
	skillComponent.addSkill(SkillType::SKILL2, std::make_shared<ProjectileSkill>(rangedAttackParams, ProjectileType::HEAL_ORB));

	entity.emplace<Milk>();
	return entity;
};
