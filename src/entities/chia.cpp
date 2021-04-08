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
	addSkill1(entity, skillComponent);
	addSkill2(entity, skillComponent);
	addSkill3(entity, skillComponent);

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

	// hitbox scaling
	auto hitboxScale = vec2({ 0.5f, 0.8f });
	motion.boundingBox = motion.scale * hitboxScale * vec2({ resource.texture.size.x, resource.texture.size.y });

	// Initialize stats
	auto& statsComponent = entity.emplace<StatsComponent>();
	json stats = configValues.at("stats");
	statsComponent.stats[StatType::HP] = stats.at("hp");
	statsComponent.stats[StatType::MAX_HP] = stats.at("hp");
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

	// hitbox scaling
	auto hitboxScale = vec2({ 0.5f, 0.8f });
	motion.boundingBox = motion.scale * hitboxScale * vec2({ resource.texture.size.x, resource.texture.size.y });

	// Initialize stats
	auto& statsComponent = entity.emplace<StatsComponent>();
	statsComponent.stats[StatType::MAX_HP] = 70.f;
	statsComponent.stats[StatType::HP] = 70.f;
	statsComponent.stats[StatType::AMBROSIA] = 0.f;
	statsComponent.stats[StatType::STRENGTH] = 1.f;

	//Add HP bar
	statsComponent.healthBar = HPBar::createHPBar({ motion.position.x, motion.position.y - 225.0f });
	ECS::registry<HPBar>.get(statsComponent.healthBar).offset = { 0.0f,-225.0f };
	ECS::registry<HPBar>.get(statsComponent.healthBar).statsCompEntity = entity;

	return entity;
}

void Chia::addSkill1(ECS::Entity entity, SkillComponent& skillComponent)
{
	// Blueberry projectile that heals allies and damages enemies

	// For the params that should be common to all levels of this skill, put them
	// in this lambda. The upgradeable params should be handled below
	auto createParams = [=]()
	{
		auto params = std::make_shared<ProjectileSkillParams>();
		params->instigator = entity;
		params->soundEffect = SoundEffect::PROJECTILE;
		params->animationType = AnimationType::ATTACK1;
		params->delay = 0.6f;
		params->entityFilters.push_back(std::make_shared<InstigatorFilter>(entity));
		params->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::PLAYER | CollisionGroup::MOB));
		params->projectileType = ProjectileType::BLUEBERRY;
		return params;
	};

	// Create as many levels of this skill as needed
	auto level1Params = createParams();
	level1Params->entityHandler = std::make_shared<HealAndDamageHandler>(CollisionGroup::PLAYER, 30.f, CollisionGroup::MOB, 20.f);
	level1Params->entityHandler->addFX(FXType::BLUEBERRIED);
	auto level1Skill = std::make_shared<ProjectileSkill>(level1Params);

	auto level2Params = createParams();
	level2Params->entityHandler = std::make_shared<HealAndDamageHandler>(CollisionGroup::PLAYER, 40.f, CollisionGroup::MOB, 30.f);
	level2Params->entityHandler->addFX(FXType::BLUEBERRIED);
	auto level2Skill = std::make_shared<ProjectileSkill>(level2Params);

	auto level3Params = createParams();
	level3Params->entityHandler = std::make_shared<HealAndDamageHandler>(CollisionGroup::PLAYER, 50.f, CollisionGroup::MOB, 40.f);
	level3Params->entityHandler->addFX(FXType::BLUEBERRIED);
	auto level3Skill = std::make_shared<ProjectileSkill>(level3Params);

	SkillLevels levels = {
			level1Skill,
			level2Skill,
			level3Skill
	};

	skillComponent.addUpgradeableSkill(SkillType::SKILL1, levels);
}

void Chia::addSkill2(ECS::Entity entity, SkillComponent& skillComponent)
{
	// Debuff the target's strength and deal damage
	// (need to click on the target you want to attack)

	// For the params that should be common to all levels of this skill, put them in
	// this lambda. The upgradeable params should be handled separately (see below)
	auto createParams = [=]()
	{
		auto params = std::make_shared<AoESkillParams>();
		params->instigator = entity;
		params->soundEffect = SoundEffect::DEBUFF;
		params->animationType = AnimationType::ATTACK2;
		params->delay = 1.f;
		params->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::MOB));
		params->entityFilters.push_back(std::make_shared<MaxTargetsFilter>(1));
		return params;
	};

	// Create as many levels of this skill as needed
	auto level1Params = createParams();
	level1Params->entityProvider = std::make_shared<MouseClickProvider>(100.f);
	level1Params->entityHandler = std::make_shared<DebuffAndDamageHandler>(StatType::STRENGTH, -0.4f, 1, 15.f);
	auto level1Skill = std::make_shared<AreaOfEffectSkill>(level1Params);

	auto level2Params = createParams();
	level2Params->entityProvider = std::make_shared<MouseClickProvider>(125.f);
	level2Params->entityHandler = std::make_shared<DebuffAndDamageHandler>(StatType::STRENGTH, -0.5f, 1, 20.f);
	auto level2Skill = std::make_shared<AreaOfEffectSkill>(level2Params);

	auto level3Params = createParams();
	level3Params->entityProvider = std::make_shared<MouseClickProvider>(150.f);
	level3Params->entityHandler = std::make_shared<DebuffAndDamageHandler>(StatType::STRENGTH, -0.6f, 1, 25.f);
	auto level3Skill = std::make_shared<AreaOfEffectSkill>(level3Params);

	SkillLevels levels = {
			level1Skill,
			level2Skill,
			level3Skill
	};

	skillComponent.addUpgradeableSkill(SkillType::SKILL2, levels);
}

void Chia::addSkill3(ECS::Entity entity, SkillComponent& skillComponent)
{
	// Grant x amount of HP shield to all allies for 1 turn

	// For the params that should be common to all levels of this skill, put them in
	// this lambda. The upgradeable params should be handled separately (see below)
	auto createParams = [=]()
	{
		auto params = std::make_shared<AoESkillParams>();
		params->instigator = entity;
		params->soundEffect = SoundEffect::BUFF;
		params->animationType = AnimationType::ATTACK3;
		params->delay = 0.6f;
		params->entityProvider = std::make_shared<AllEntitiesProvider>();
		params->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::PLAYER));
		return params;
	};

	// Create as many levels of this skill as needed
	auto level1Params = createParams();
	level1Params->entityHandler = std::make_shared<BuffHandler>(StatType::HP_SHIELD, 30.f, 1);
	auto level1Skill = std::make_shared<AreaOfEffectSkill>(level1Params);

	auto level2Params = createParams();
	level2Params->entityHandler = std::make_shared<BuffHandler>(StatType::HP_SHIELD, 40.f, 1);
	auto level2Skill = std::make_shared<AreaOfEffectSkill>(level2Params);

	auto level3Params = createParams();
	level3Params->entityHandler = std::make_shared<BuffHandler>(StatType::HP_SHIELD, 50.f, 2);
	auto level3Skill = std::make_shared<AreaOfEffectSkill>(level3Params);

	SkillLevels levels = {
			level1Skill,
			level2Skill,
			level3Skill
	};

	skillComponent.addUpgradeableSkill(SkillType::SKILL3, levels);
}
