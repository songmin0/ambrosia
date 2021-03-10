#include "chia.hpp"

#include "rendering/render.hpp"
#include "animation/animation_components.hpp"
#include "game/turn_system.hpp"
#include "ui/ui_entities.hpp"

ECS::Entity Chia::createChia(json configValues)
{
	auto entity = ECS::Entity();

	std::string key = "chia_static";
	ShadedMesh& resource = cacheResource(key);
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, spritePath("players/chia/chia_static.png"), "textured");
	}

	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<RenderableComponent>(RenderLayer::PLAYER_AND_MOB);

	// Setting initial motion values
	Motion& motion = entity.emplace<Motion>();
	motion.position = vec2(configValues.at("position")[0], configValues.at("position")[1]);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2(1.f);
	motion.colliderType = CollisionGroup::PLAYER;
	motion.collidesWith = CollisionGroup::MOB;

	// hitbox scaling
	auto hitboxScale = vec2({ 0.5f, 0.8f });
	motion.boundingBox = motion.scale * hitboxScale * vec2({ resource.texture.size.x, resource.texture.size.y });

	// Animations
	auto idle_anim = new AnimationData("chia_idle", spritePath("players/chia/idle/idle"), 61);
	AnimationsComponent& anims = entity.emplace<AnimationsComponent>(AnimationType::IDLE, *idle_anim);

	auto move_anim = new AnimationData("chia_move", spritePath("players/chia/move/move"), 32);
	anims.addAnimation(AnimationType::MOVE, *move_anim);

	auto hit = new AnimationData("chia_hit", spritePath("players/chia/hit/hit"), 34, 1, true, false);
	anims.addAnimation(AnimationType::HIT, *hit);

	auto defeat = new AnimationData("chia_defeat", spritePath("players/chia/defeat/defeat"), 41, 1, true, false);
	anims.addAnimation(AnimationType::DEFEAT, *defeat);

	auto attack1 = new AnimationData("chia_attack1", spritePath("players/chia/attack1/attack1"), 41, 1, true, false, vec2({ 0.08f, 0.f }));
	anims.addAnimation(AnimationType::ATTACK1, *attack1);

	auto attack2 = new AnimationData("chia_attack2", spritePath("players/chia/attack2/attack2"), 64, 1, true, false, vec2({ -0.f, 0.04f }));
	anims.addAnimation(AnimationType::ATTACK2, *attack2);

	auto attack3 = new AnimationData("chia_attack3", spritePath("players/chia/attack3/attack3"), 64, 1, true, false, vec2({ -0.f, 0.04f }));
	anims.addAnimation(AnimationType::ATTACK3, *attack3);

	entity.emplace<Chia>();

	//Add the turn component
	entity.emplace<TurnSystem::TurnComponent>();

	//Add the player component
	entity.emplace<PlayerComponent>().player = PlayerType::CHIA;

	// Initialize stats
	auto& statsComponent = entity.emplace<StatsComponent>();
	json stats = configValues.at("stats");
	statsComponent.stats[StatType::HP] = stats.at("hp");
	statsComponent.stats[StatType::AMBROSIA] = stats.at("ambrosia");
	statsComponent.stats[StatType::STRENGTH] = stats.at("strength");

	// Initialize skills
	auto& skillComponent = entity.emplace<SkillComponent>();

	//Add HP bar
	statsComponent.healthBar = HPBar::createHPBar({ motion.position.x, motion.position.y - 225.0f });
	ECS::registry<HPBar>.get(statsComponent.healthBar).offset = { 0.0f,-225.0f };
	ECS::registry<HPBar>.get(statsComponent.healthBar).statsCompEntity = entity;

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

	// Strength buff for nearby players (the instigator will also be buffed)
	StatModifier strengthBuffModifier;
	strengthBuffModifier.statType = StatType::STRENGTH;
	strengthBuffModifier.value = 0.1f;
	strengthBuffModifier.timer = 60.f;

	SkillParams strengthBuffParams;
	strengthBuffParams.instigator = entity;
	strengthBuffParams.animationType = AnimationType::ATTACK2;
	strengthBuffParams.delay = 1.f;
	strengthBuffParams.range = 300.f;
	strengthBuffParams.ignoreInstigator = false;
	strengthBuffParams.collideWithMultipleEntities = true;
	strengthBuffParams.collidesWith = CollisionGroup::PLAYER;
	strengthBuffParams.soundEffect = SoundEffect::BUFF;
	skillComponent.addSkill(SkillType::SKILL2, std::make_shared<BuffProximitySkill>(strengthBuffParams, strengthBuffModifier));

	// Bone throw projectile attack
	SkillParams boneThrowParams;
	boneThrowParams.instigator = entity;
	boneThrowParams.animationType = AnimationType::ATTACK3;
	boneThrowParams.delay = 0.6f;
	boneThrowParams.damage = 20.f;
	boneThrowParams.collidesWith = CollisionGroup::MOB;
	boneThrowParams.soundEffect = SoundEffect::PROJECTILE;
	skillComponent.addSkill(SkillType::SKILL3, std::make_shared<ProjectileSkill>(boneThrowParams, ProjectileType::BONE));

	// Placeholder, just for fun, debuff an enemy by clicking on the enemy (doesn't matter how far away they are from the player)
	StatModifier strengthDebuffModifier;
	strengthDebuffModifier.statType = StatType::STRENGTH;
	strengthDebuffModifier.value = -0.1f;
	strengthDebuffModifier.timer = 60.f;

	SkillParams strengthDebuffParams;
	strengthDebuffParams.instigator = entity;
	strengthDebuffParams.animationType = AnimationType::ATTACK2; // TEMPORARILY SETTING TO ATTACK2 ANIMATION
	strengthDebuffParams.delay = 1.f;
	strengthDebuffParams.range = 100.f;
	strengthDebuffParams.ignoreInstigator = true;
	strengthDebuffParams.collideWithMultipleEntities = false;
	strengthDebuffParams.collidesWith = CollisionGroup::MOB;
	strengthDebuffParams.soundEffect = SoundEffect::DEBUFF;
	skillComponent.addSkill(SkillType::SKILL4, std::make_shared<BuffMouseClickSkill>(strengthDebuffParams, strengthDebuffModifier));

	return entity;
};

ECS::Entity Chia::createChia(vec2 position)
{
	auto entity = ECS::Entity();

	std::string key = "chia_static";
	ShadedMesh& resource = cacheResource(key);
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, spritePath("players/chia/chia_static.png"), "textured");
	}

	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<RenderableComponent>(RenderLayer::PLAYER_AND_MOB);

	// Setting initial motion values
	Motion& motion = entity.emplace<Motion>();
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2(1.f);
	motion.colliderType = CollisionGroup::PLAYER;
	motion.collidesWith = CollisionGroup::MOB;

	// hitbox scaling
	auto hitboxScale = vec2({ 0.5f, 0.8f });
	motion.boundingBox = motion.scale * hitboxScale * vec2({ resource.texture.size.x, resource.texture.size.y });

	// Animations
	auto idle_anim = new AnimationData(
		"chia_idle", spritePath("players/chia/idle/idle"), 61);
	AnimationsComponent& anims = entity.emplace<AnimationsComponent>(AnimationType::IDLE, *idle_anim);

	auto move_anim = new AnimationData(
		"chia_move", spritePath("players/chia/move/move"), 32);
	anims.addAnimation(AnimationType::MOVE, *move_anim);

	entity.emplace<Chia>();

	//Add the turn component
	entity.emplace<TurnSystem::TurnComponent>();

	//Add the player component
	entity.emplace<PlayerComponent>().player = PlayerType::CHIA;

	// Initialize stats
	auto& statsComponent = entity.emplace<StatsComponent>();
	statsComponent.stats[StatType::HP] = 100.f;
	statsComponent.stats[StatType::AMBROSIA] = 0.f;
	statsComponent.stats[StatType::STRENGTH] = 1.f;

	//Add HP bar
	statsComponent.healthBar = HPBar::createHPBar({ motion.position.x, motion.position.y - 225.0f });
	ECS::registry<HPBar>.get(statsComponent.healthBar).offset = { 0.0f,-225.0f };
	ECS::registry<HPBar>.get(statsComponent.healthBar).statsCompEntity = entity;

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

	// Strength buff for nearby players (the instigator will also be buffed)
	StatModifier strengthBuffModifier;
	strengthBuffModifier.statType = StatType::STRENGTH;
	strengthBuffModifier.value = 0.1f;
	strengthBuffModifier.timer = 60.f;

	SkillParams strengthBuffParams;
	strengthBuffParams.instigator = entity;
	strengthBuffParams.animationType = AnimationType::ATTACK2;
	strengthBuffParams.delay = 1.f;
	strengthBuffParams.range = 300.f;
	strengthBuffParams.ignoreInstigator = false;
	strengthBuffParams.collideWithMultipleEntities = true;
	strengthBuffParams.collidesWith = CollisionGroup::PLAYER;
	strengthBuffParams.soundEffect = SoundEffect::BUFF;
	skillComponent.addSkill(SkillType::SKILL2, std::make_shared<BuffProximitySkill>(strengthBuffParams, strengthBuffModifier));

	// Bone throw projectile attack
	SkillParams boneThrowParams;
	boneThrowParams.instigator = entity;
	boneThrowParams.animationType = AnimationType::ATTACK3;
	boneThrowParams.delay = 0.6f;
	boneThrowParams.damage = 20.f;
	boneThrowParams.collidesWith = CollisionGroup::MOB;
	boneThrowParams.soundEffect = SoundEffect::PROJECTILE;
	skillComponent.addSkill(SkillType::SKILL3, std::make_shared<ProjectileSkill>(boneThrowParams, ProjectileType::BONE));

	// Placeholder, just for fun, debuff an enemy by clicking on the enemy (doesn't matter how far away they are from the player)
	StatModifier strengthDebuffModifier;
	strengthDebuffModifier.statType = StatType::STRENGTH;
	strengthDebuffModifier.value = -0.1f;
	strengthDebuffModifier.timer = 60.f;

	SkillParams strengthDebuffParams;
	strengthDebuffParams.instigator = entity;
	strengthDebuffParams.animationType = AnimationType::ATTACK2; // TEMPORARILY SETTING TO ATTACK2 ANIMATION
	strengthDebuffParams.delay = 1.f;
	strengthDebuffParams.range = 100.f;
	strengthDebuffParams.ignoreInstigator = true;
	strengthDebuffParams.collideWithMultipleEntities = false;
	strengthDebuffParams.collidesWith = CollisionGroup::MOB;
	strengthDebuffParams.soundEffect = SoundEffect::DEBUFF;
	skillComponent.addSkill(SkillType::SKILL4, std::make_shared<BuffMouseClickSkill>(strengthDebuffParams, strengthDebuffModifier));

	return entity;
};
