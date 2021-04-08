#include "taji.hpp"

#include "rendering/render.hpp"
#include "animation/animation_components.hpp"
#include "game/turn_system.hpp"
#include "ui/ui_entities.hpp"

ECS::Entity Taji::commonInit()
{
	auto entity = ECS::Entity();
	entity.emplace<RenderableComponent>(RenderLayer::PLAYER_AND_MOB);

	// Animations
	auto idle_anim = AnimationData("taji_idle", spritePath("players/taji/idle/idle"), 62);
	AnimationsComponent& anims = entity.emplace<AnimationsComponent>(AnimationType::IDLE, std::make_shared<AnimationData>(idle_anim));

	auto move_anim = AnimationData("taji_move", spritePath("players/taji/move/move"), 32);
	anims.addAnimation(AnimationType::MOVE, std::make_shared<AnimationData>(move_anim));

	auto hit = AnimationData("taji_hit", spritePath("players/taji/hit/hit"), 49, 1, true, false);
	anims.addAnimation(AnimationType::HIT, std::make_shared<AnimationData>(hit));

	auto defeat = AnimationData("taji_defeat", spritePath("players/taji/defeat/defeat"), 61, 1, true, false);
	anims.addAnimation(AnimationType::DEFEAT, std::make_shared<AnimationData>(defeat));

	auto attack1 = AnimationData("taji_attack1", spritePath("players/taji/attack1/attack1"), 55, 1, true, false);
	anims.addAnimation(AnimationType::ATTACK1, std::make_shared<AnimationData>(attack1));

	auto attack2 = AnimationData("taji_attack2", spritePath("players/taji/attack2/attack2"), 55, 1, true, false);
	anims.addAnimation(AnimationType::ATTACK2, std::make_shared<AnimationData>(attack2));

	auto attack3 = AnimationData("taji_attack3", spritePath("players/taji/attack3/attack3"), 83, 1, true, false);
	anims.addAnimation(AnimationType::ATTACK3, std::make_shared<AnimationData>(attack3));

	// Player and Turn Component
	entity.emplace<TurnSystem::TurnComponent>();
	entity.emplace<PlayerComponent>().player = PlayerType::TAJI;

	// Initialize skills
	auto& skillComponent = entity.emplace<SkillComponent>();
	addSkill1(entity, skillComponent);
	addSkill2(entity, skillComponent);
	addSkill3(entity, skillComponent);

	entity.emplace<Taji>();
	return entity;
}

ECS::Entity Taji::createTaji(json configValues)
{
	auto entity = commonInit();

	ShadedMesh& resource = cacheResource("taji_static");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, spritePath("players/taji/taji_static.png"), "textured");
	}
	entity.emplace<ShadedMeshRef>(resource);

	// Setting initial motion values
	Motion& motion = entity.emplace<Motion>();
	motion.position = vec2(configValues.at("position")[0], configValues.at("position")[1]);
	motion.scale = vec2({ 0.97f, 0.97f });
	motion.colliderType = CollisionGroup::PLAYER;

	// hitbox scaling
	auto hitboxScale = vec2({ 0.5f, 0.85f });
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

ECS::Entity Taji::createTaji(vec2 position)
{
	auto entity = commonInit();

	ShadedMesh& resource = cacheResource("taji_static");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, spritePath("players/taji/taji_static.png"), "textured");
	}
	entity.emplace<ShadedMeshRef>(resource);

	// Setting initial motion values
	Motion& motion = entity.emplace<Motion>();
	motion.position = position;
	motion.scale = vec2({ 0.97f, 0.97f });
	motion.colliderType = CollisionGroup::PLAYER;

	// hitbox scaling
	auto hitboxScale = vec2({ 0.5f, 0.85f });
	motion.boundingBox = motion.scale * hitboxScale * vec2({ resource.texture.size.x, resource.texture.size.y });

	// Initialize stats
	auto& statsComponent = entity.emplace<StatsComponent>();
	statsComponent.stats[StatType::MAX_HP] = 60.f;
	statsComponent.stats[StatType::HP] = 60.f;
	statsComponent.stats[StatType::AMBROSIA] = 0.f;
	statsComponent.stats[StatType::STRENGTH] = 1.f;

	//Add HP bar
	statsComponent.healthBar = HPBar::createHPBar({ motion.position.x, motion.position.y - 225.0f });
	ECS::registry<HPBar>.get(statsComponent.healthBar).offset = { 0.0f,-225.0f };
	ECS::registry<HPBar>.get(statsComponent.healthBar).statsCompEntity = entity;

	return entity;
};

void Taji::addSkill1(ECS::Entity entity, SkillComponent& skillComponent)
{
	// Ranged damage with small area of effect

	// For the params that should be common to all levels of this skill, put them
	// in this lambda. The upgradeable params should be handled below
	auto createParams = [&]()
	{
		auto params = std::make_shared<AoESkillParams>();
		params->instigator = entity;
		params->soundEffect = SoundEffect::BUFF;
		params->animationType = AnimationType::ATTACK1;
		params->delay = 1.f;
		params->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::MOB));
		return params;
	};

	// Create as many levels of this skill as needed
	auto level1Params = createParams();
	level1Params->entityProvider = std::make_shared<MouseClickProvider>(250.f);
	level1Params->entityHandler = std::make_shared<DamageHandler>(20.f);
	level1Params->entityHandler->addFX(FXType::CANDY1);
	auto level1Skill = std::make_shared<AreaOfEffectSkill>(level1Params);

	auto level2Params = createParams();
	level2Params->entityProvider = std::make_shared<MouseClickProvider>(275.f);
	level2Params->entityHandler = std::make_shared<DamageHandler>(25.f);
	level2Params->entityHandler->addFX(FXType::CANDY1);
	auto level2Skill = std::make_shared<AreaOfEffectSkill>(level2Params);

	auto level3Params = createParams();
	level3Params->entityProvider = std::make_shared<MouseClickProvider>(300.f);
	level3Params->entityHandler = std::make_shared<DamageHandler>(30.f);
	level3Params->entityHandler->addFX(FXType::CANDY1);
	auto level3Skill = std::make_shared<AreaOfEffectSkill>(level3Params);

	SkillLevels levels = {
			level1Skill,
			level2Skill,
			level3Skill
	};

	skillComponent.addUpgradeableSkill(SkillType::SKILL1, levels);
}

void Taji::addSkill2(ECS::Entity entity, SkillComponent& skillComponent)
{
	// Single-target ranged attack, deals damage and makes the target skip a turn

	// For the params that should be common to all levels of this skill, put them in
	// this lambda. The upgradeable params should be handled separately (see below)
	auto createParams = [=]()
	{
		auto params = std::make_shared<AoESkillParams>();
		params->instigator = entity;
		params->soundEffect = SoundEffect::BUFF;
		params->animationType = AnimationType::ATTACK2;
		params->delay = 1.f;
		params->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::MOB));
		params->entityFilters.push_back(std::make_shared<MaxTargetsFilter>(1));
		return params;
	};

	// Create as many levels of this skill as needed
	auto level1Params = createParams();
	level1Params->entityProvider = std::make_shared<MouseClickProvider>(100.f);
	level1Params->entityHandler = std::make_shared<DebuffAndDamageHandler>(StatType::STUNNED, 1.f, 1, 20.f);
	level1Params->entityHandler->addFX(FXType::CANDY2);
	auto level1Skill = std::make_shared<AreaOfEffectSkill>(level1Params);

	auto level2Params = createParams();
	level2Params->entityProvider = std::make_shared<MouseClickProvider>(125.f);
	level2Params->entityHandler = std::make_shared<DebuffAndDamageHandler>(StatType::STUNNED, 1.f, 1, 30.f);
	level2Params->entityHandler->addFX(FXType::CANDY2);
	auto level2Skill = std::make_shared<AreaOfEffectSkill>(level2Params);

	auto level3Params = createParams();
	level3Params->entityProvider = std::make_shared<MouseClickProvider>(150.f);
	level3Params->entityHandler = std::make_shared<DebuffAndDamageHandler>(StatType::STUNNED, 1.f, 2, 40.f);
	level3Params->entityHandler->addFX(FXType::CANDY2);
	auto level3Skill = std::make_shared<AreaOfEffectSkill>(level3Params);

	SkillLevels levels = {
			level1Skill,
			level2Skill,
			level3Skill
	};

	skillComponent.addUpgradeableSkill(SkillType::SKILL2, levels);
}

void Taji::addSkill3(ECS::Entity entity, SkillComponent& skillComponent)
{
	// Small damage to all enemies, small heal to all allies

	// For the params that should be common to all levels of this skill, put them in
	// this lambda. The upgradeable params should be handled separately (see below)
	auto createParams = [=]()
	{
		auto params = std::make_shared<AoESkillParams>();
		params->instigator = entity;
		params->soundEffect = SoundEffect::BUFF;
		params->animationType = AnimationType::ATTACK3;
		params->delay = 1.f;
		params->entityProvider = std::make_shared<AllEntitiesProvider>();
		params->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::PLAYER | CollisionGroup::MOB));
		return params;
	};

	// Create as many levels of this skill as needed
	auto level1Params = createParams();
	level1Params->entityHandler = std::make_shared<HealAndDamageHandler>(CollisionGroup::PLAYER, 8.f, CollisionGroup::MOB, 8.f);
	auto level1Skill = std::make_shared<AreaOfEffectSkill>(level1Params);

	auto level2Params = createParams();
	level2Params->entityHandler = std::make_shared<HealAndDamageHandler>(CollisionGroup::PLAYER, 12.f, CollisionGroup::MOB, 12.f);
	auto level2Skill = std::make_shared<AreaOfEffectSkill>(level2Params);

	auto level3Params = createParams();
	level3Params->entityHandler = std::make_shared<HealAndDamageHandler>(CollisionGroup::PLAYER, 16.f, CollisionGroup::MOB, 16.f);
	auto level3Skill = std::make_shared<AreaOfEffectSkill>(level3Params);

	SkillLevels levels = {
			level1Skill,
			level2Skill,
			level3Skill
	};

	skillComponent.addUpgradeableSkill(SkillType::SKILL3, levels);
}
