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
	ECS::registry<HPBar>.get(statsComponent.healthBar).offset = { 0.0f, -250.0f };
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

ECS::Entity Potato::createPotato(vec2 pos)
{
	auto entity = ECS::Entity();

	ShadedMesh& resource = cacheResource("potato_static");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, spritePath("enemies/potato/potato_static.png"), "textured");
	}
	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<RenderableComponent>(RenderLayer::PLAYER_AND_MOB);

	// TODO: Figure this out for Potato
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
	motion.scale = vec2(1.4f);
	motion.orientation = -1;
	auto hitboxScale = vec2({ 0.7f, 1.f });
	motion.boundingBox = motion.scale * hitboxScale * vec2({ resource.texture.size.x, resource.texture.size.y });
	motion.colliderType = CollisionGroup::MOB;
	motion.collidesWith = CollisionGroup::PLAYER;

	// Animations
	auto idle = new AnimationData("potato_idle", spritePath("enemies/potato/idle/idle"), 43);
	AnimationsComponent& anims = entity.emplace<AnimationsComponent>(AnimationType::IDLE, *idle);

	auto move = new AnimationData("potato_move", spritePath("enemies/potato/move/move"), 36);
	anims.addAnimation(AnimationType::MOVE, *move);

	auto hit_anim = new AnimationData("potato_hit", spritePath("enemies/potato/hit/hit"), 16, 1, true, false);
	anims.addAnimation(AnimationType::HIT, *hit_anim);

	auto attack1_anim = new AnimationData("potato_attack1", spritePath("enemies/potato/attack1/attack1"), 30, 1, true, false, { -0.02f, 0.f });
	anims.addAnimation(AnimationType::ATTACK1, *attack1_anim);

	auto attack2_anim = new AnimationData("potato_attack2", spritePath("enemies/potato/attack2/attack2"), 41, 1, true, false, { 0.02f, 0.22f });
	anims.addAnimation(AnimationType::ATTACK2, *attack2_anim);

	auto defeat_anim = new AnimationData("potato_defeat", spritePath("enemies/potato/defeat/defeat"), 47, 1, true, false, { 0.02f, 0.22f });
	anims.addAnimation(AnimationType::DEFEAT, *defeat_anim);

	// TODO: Create MaxHP stat. A boss shouldn't have the same max HP as a mob.
	// Initialize stats
	auto& statsComponent = entity.emplace<StatsComponent>();
	statsComponent.stats[StatType::HP] = 100.f;
	statsComponent.stats[StatType::AMBROSIA] = 0.f;
	statsComponent.stats[StatType::STRENGTH] = 1.f;

	//Add HP bar
	statsComponent.healthBar = HPBar::createHPBar({ motion.position.x, motion.position.y - 150.0f }, { 1.f, 0.55f });
	ECS::registry<HPBar>.get(statsComponent.healthBar).offset = { 0.0f, -380.0f };
	ECS::registry<HPBar>.get(statsComponent.healthBar).statsCompEntity = entity;

	// Initialize skills
	auto& skillComponent = entity.emplace<SkillComponent>();

	// Skill 1, single-target melee hit
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

	// Skill 2, large aoe blast
	SkillParams aoeParams;
	aoeParams.instigator = entity;
	aoeParams.animationType = AnimationType::ATTACK2;
	aoeParams.delay = 1.f;
	aoeParams.damage = 60.f;
	aoeParams.range = 600.f;
	aoeParams.collideWithMultipleEntities = true;
	aoeParams.collidesWith = CollisionGroup::PLAYER;
	aoeParams.soundEffect = SoundEffect::MELEE;
	skillComponent.addSkill(SkillType::SKILL2, std::make_shared<MeleeSkill>(aoeParams));

	entity.emplace<Potato>();
	return entity;
};
