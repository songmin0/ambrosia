#include "chia.hpp"

#include "rendering/render.hpp"
#include "animation/animation_components.hpp"
#include "game/turn_system.hpp"
#include "ui/ui_entities.hpp"

ECS::Entity Chia::commonInit()
{
	auto entity = ECS::Entity();
	entity.emplace<RenderableComponent>(RenderLayer::PLAYER_AND_MOB);

	// Animations
	auto idle_anim = AnimationData("chia_idle", spritePath("players/chia/idle/idle"), 61);
	AnimationsComponent& anims = entity.emplace<AnimationsComponent>(AnimationType::IDLE, std::make_shared<AnimationData>(idle_anim));

	auto move_anim = AnimationData("chia_move", spritePath("players/chia/move/move"), 32);
	anims.addAnimation(AnimationType::MOVE, std::make_shared<AnimationData>(move_anim));

	auto hit = AnimationData("chia_hit", spritePath("players/chia/hit/hit"), 34, 1, true, false);
	anims.addAnimation(AnimationType::HIT, std::make_shared<AnimationData>(hit));

	auto defeat = AnimationData("chia_defeat", spritePath("players/chia/defeat/defeat"), 41, 1, true, false);
	anims.addAnimation(AnimationType::DEFEAT, std::make_shared<AnimationData>(defeat));

	auto attack1 = AnimationData("chia_attack1", spritePath("players/chia/attack1/attack1"), 41, 1, true, false, vec2({ 0.08f, 0.f }));
	anims.addAnimation(AnimationType::ATTACK1, std::make_shared<AnimationData>(attack1));

	auto attack2 = AnimationData("chia_attack2", spritePath("players/chia/attack2/attack2"), 64, 1, true, false, vec2({ -0.f, 0.04f }));
	anims.addAnimation(AnimationType::ATTACK2, std::make_shared<AnimationData>(attack2));

	auto attack3 = AnimationData("chia_attack3", spritePath("players/chia/attack3/attack3"), 64, 1, true, false, vec2({ -0.f, 0.04f }));
	anims.addAnimation(AnimationType::ATTACK3, std::make_shared<AnimationData>(attack3));

	// Player and Turn Components
	entity.emplace<TurnSystem::TurnComponent>();
	entity.emplace<PlayerComponent>().player = PlayerType::CHIA;

	// Initialize skills
	auto& skillComponent = entity.emplace<SkillComponent>();

	// Melee hit
	SkillParams meleeParams;
	meleeParams.instigator = entity;
	meleeParams.animationType = AnimationType::ATTACK1;
	meleeParams.delay = 1.f;
	meleeParams.damage = 30.f;
	meleeParams.range = 200.f;
	meleeParams.collideWithMultipleEntities = false;
	meleeParams.collidesWith = CollisionGroup::MOB;
	meleeParams.soundEffect = SoundEffect::MELEE;
	skillComponent.addSkill(SkillType::SKILL1, std::make_shared<MeleeSkill>(meleeParams));

	// Debuff an enemy by clicking on the enemy (doesn't matter how far away they are from the player)
	StatModifier strengthDebuffModifier;
	strengthDebuffModifier.statType = StatType::STRENGTH;
	strengthDebuffModifier.value = -0.1f;
	strengthDebuffModifier.timer = 60.f;

	SkillParams strengthDebuffParams;
	strengthDebuffParams.instigator = entity;
	strengthDebuffParams.animationType = AnimationType::ATTACK2;
	strengthDebuffParams.delay = 1.f;
	strengthDebuffParams.range = 100.f;
	strengthDebuffParams.ignoreInstigator = true;
	strengthDebuffParams.collideWithMultipleEntities = false;
	strengthDebuffParams.collidesWith = CollisionGroup::MOB;
	strengthDebuffParams.soundEffect = SoundEffect::DEBUFF;
	skillComponent.addSkill(SkillType::SKILL2, std::make_shared<BuffMouseClickSkill>(strengthDebuffParams, strengthDebuffModifier));

	// Bone throw projectile attack
	SkillParams boneThrowParams;
	boneThrowParams.instigator = entity;
	boneThrowParams.animationType = AnimationType::ATTACK3;
	boneThrowParams.delay = 0.6f;
	boneThrowParams.damage = 20.f;
	boneThrowParams.collidesWith = CollisionGroup::MOB;
	boneThrowParams.soundEffect = SoundEffect::PROJECTILE;
	skillComponent.addSkill(SkillType::SKILL3, std::make_shared<ProjectileSkill>(boneThrowParams, ProjectileType::BONE));

	entity.emplace<Chia>();
	return entity;
}

ECS::Entity Chia::createChia(json configValues)
{
	auto entity = commonInit();

	ShadedMesh& resource = cacheResource("chia_static");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, spritePath("players/chia/chia_static.png"), "textured");
	}
	entity.emplace<ShadedMeshRef>(resource);

	// Setting initial motion values
	Motion& motion = entity.emplace<Motion>();
	motion.position = vec2(configValues.at("position")[0], configValues.at("position")[1]);
	motion.colliderType = CollisionGroup::PLAYER;
	motion.collidesWith = CollisionGroup::MOB;

	// hitbox scaling
	auto hitboxScale = vec2({ 0.5f, 0.8f });
	motion.boundingBox = motion.scale * hitboxScale * vec2({ resource.texture.size.x, resource.texture.size.y });

	// Initialize stats
	auto& statsComponent = entity.emplace<StatsComponent>();
	json stats = configValues.at("stats");
	statsComponent.stats[StatType::HP] = stats.at("hp");
	statsComponent.stats[StatType::MAXHP] = stats.at("hp");
	statsComponent.stats[StatType::AMBROSIA] = stats.at("ambrosia");
	statsComponent.stats[StatType::STRENGTH] = stats.at("strength");

	//Add HP bar
	statsComponent.healthBar = HPBar::createHPBar({ motion.position.x, motion.position.y - 225.0f });
	ECS::registry<HPBar>.get(statsComponent.healthBar).offset = { 0.0f,-225.0f };
	ECS::registry<HPBar>.get(statsComponent.healthBar).statsCompEntity = entity;

	return entity;
};

ECS::Entity Chia::createChia(vec2 position)
{
	auto entity = commonInit();

	ShadedMesh& resource = cacheResource("chia_static");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, spritePath("players/chia/chia_static.png"), "textured");
	}
	entity.emplace<ShadedMeshRef>(resource);


	// Setting initial motion values
	Motion& motion = entity.emplace<Motion>();
	motion.position = position;
	motion.colliderType = CollisionGroup::PLAYER;
	motion.collidesWith = CollisionGroup::MOB;

	// hitbox scaling
	auto hitboxScale = vec2({ 0.5f, 0.8f });
	motion.boundingBox = motion.scale * hitboxScale * vec2({ resource.texture.size.x, resource.texture.size.y });

	// Initialize stats
	auto& statsComponent = entity.emplace<StatsComponent>();
	statsComponent.stats[StatType::MAXHP] = 70.f;
	statsComponent.stats[StatType::HP] = 70.f;
	statsComponent.stats[StatType::AMBROSIA] = 0.f;
	statsComponent.stats[StatType::STRENGTH] = 1.f;

	//Add HP bar
	statsComponent.healthBar = HPBar::createHPBar({ motion.position.x, motion.position.y - 225.0f });
	ECS::registry<HPBar>.get(statsComponent.healthBar).offset = { 0.0f,-225.0f };
	ECS::registry<HPBar>.get(statsComponent.healthBar).statsCompEntity = entity;

	return entity;
};
