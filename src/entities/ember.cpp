#include "ember.hpp"

#include "rendering/render.hpp"
#include "animation/animation_components.hpp"
#include "game/turn_system.hpp"
#include "ui/ui_entities.hpp"

ECS::Entity Ember::commonInit()
{
	auto entity = ECS::Entity();
	entity.emplace<RenderableComponent>(RenderLayer::PLAYER_AND_MOB);

	// Animations
	auto idle_anim = AnimationData("ember_idle", spritePath("players/ember/idle/idle"), 60);
	AnimationsComponent& anims = entity.emplace<AnimationsComponent>(AnimationType::IDLE, std::make_shared<AnimationData>(idle_anim));

	auto move_anim = AnimationData("ember_move", spritePath("players/ember/move/move"), 32);
	anims.addAnimation(AnimationType::MOVE, std::make_shared<AnimationData>(move_anim));

	auto attack1 = AnimationData("ember_attack1", spritePath("players/ember/attack1/attack1"), 50, 1, true, false, vec2({ -0.02f, 0.37f }));
	anims.addAnimation(AnimationType::ATTACK1, std::make_shared<AnimationData>(attack1));

	auto attack2 = AnimationData("ember_attack2", spritePath("players/ember/attack2/attack2"), 61, 1, true, false, vec2({ -0.02f, 0.37f }));
	anims.addAnimation(AnimationType::ATTACK2, std::make_shared<AnimationData>(attack2));

	auto attack3 = AnimationData("ember_attack3", spritePath("players/ember/attack3/attack3"), 61, 1, true, false, vec2({ -0.02f, 0.3f }));
	anims.addAnimation(AnimationType::ATTACK3, std::make_shared<AnimationData>(attack3));

	auto defeat = AnimationData("ember_defeat", spritePath("players/ember/defeat/defeat"), 57, 1, true, false, vec2({ -0.1f, 0.06f }));
	anims.addAnimation(AnimationType::DEFEAT, std::make_shared<AnimationData>(defeat));

	auto hit = AnimationData("ember_hit", spritePath("players/ember/hit/hit"), 34, 1, true, false);
	anims.addAnimation(AnimationType::HIT, std::make_shared<AnimationData>(hit));

	// Player and Turn Component
	entity.emplace<TurnSystem::TurnComponent>();
	entity.emplace<PlayerComponent>().player = PlayerType::EMBER;

	// Initialize skills
	auto& skillComponent = entity.emplace<SkillComponent>();
	addSkill1(entity, skillComponent);
	addSkill2(entity, skillComponent);
	addSkill3(entity, skillComponent);

	entity.emplace<Ember>();
	return entity;
};

ECS::Entity Ember::createEmber(json configValues)
{
	auto entity = commonInit();

	ShadedMesh& resource = cacheResource("ember_static");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, spritePath("players/ember/ember_static.png"), "textured");
	}
	entity.emplace<ShadedMeshRef>(resource);

	// Setting initial motion values
	Motion& motion = entity.emplace<Motion>();
	motion.position = vec2(configValues.at("position")[0], configValues.at("position")[1]);
	motion.colliderType = CollisionGroup::PLAYER;

	// hitbox scaling
	auto hitboxScale = vec2({ 0.4f, 0.6f });
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

ECS::Entity Ember::createEmber(vec2 position)
{
	auto entity = commonInit();

	ShadedMesh& resource = cacheResource("ember_static");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, spritePath("players/ember/ember_static.png"), "textured");
	}
	entity.emplace<ShadedMeshRef>(resource);

	// Setting initial motion values
	Motion& motion = entity.emplace<Motion>();
	motion.position = position;
	motion.colliderType = CollisionGroup::PLAYER;

	// hitbox scaling
	auto hitboxScale = vec2({ 0.4f, 0.6f });
	motion.boundingBox = motion.scale * hitboxScale * vec2({ resource.texture.size.x, resource.texture.size.y });

	// Initialize stats
	auto& statsComponent = entity.emplace<StatsComponent>();
	statsComponent.stats[StatType::MAX_HP] = 80.f;
	statsComponent.stats[StatType::HP] = 80.f;
	statsComponent.stats[StatType::AMBROSIA] = 0.f;
	statsComponent.stats[StatType::STRENGTH] = 1.f;

	//Add HP bar
	statsComponent.healthBar = HPBar::createHPBar({ motion.position.x, motion.position.y - 225.0f });
	ECS::registry<HPBar>.get(statsComponent.healthBar).offset = { 0.0f,-225.0f };
	ECS::registry<HPBar>.get(statsComponent.healthBar).statsCompEntity = entity;

	return entity;
};

void Ember::addSkill1(ECS::Entity entity, SkillComponent& skillComponent)
{
	// Melee hit

	// For the params that should be common to all levels of this skill, put them
	// in this lambda. The upgradeable params should be handled below
	auto createParams = [=]()
	{
		auto params = std::make_shared<AoESkillParams>();
		params->instigator = entity;
		params->soundEffect = SoundEffect::MELEE;
		params->animationType = AnimationType::ATTACK1;
		params->delay = 1.f;
		params->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::MOB));
		return params;
	};

	// Create as many levels of this skill as needed
	auto level1Params = createParams();
	level1Params->entityProvider = std::make_shared<CircularProvider>(300.f);
	level1Params->entityHandler = std::make_shared<DamageHandler>(25.f);
	auto level1Skill = std::make_shared<AreaOfEffectSkill>(level1Params);

	auto level2Params = createParams();
	level2Params->entityProvider = std::make_shared<CircularProvider>(325.f);
	level2Params->entityHandler = std::make_shared<DamageHandler>(30.f);
	auto level2Skill = std::make_shared<AreaOfEffectSkill>(level2Params);

	auto level3Params = createParams();
	level3Params->entityProvider = std::make_shared<CircularProvider>(350.f);
	level3Params->entityHandler = std::make_shared<DamageHandler>(35.f);
	auto level3Skill = std::make_shared<AreaOfEffectSkill>(level3Params);

	SkillLevels levels = {
			level1Skill,
			level2Skill,
			level3Skill
	};

	skillComponent.addUpgradeableSkill(SkillType::SKILL1, levels);
}

void Ember::addSkill2(ECS::Entity entity, SkillComponent& skillComponent)
{
	// Melee hit

	// For the params that should be common to all levels of this skill, put them in
	// this lambda. The upgradeable params should be handled separately (see below)
	auto createParams = [=]()
	{
		auto params = std::make_shared<AoESkillParams>();
		params->instigator = entity;
		params->soundEffect = SoundEffect::MELEE;
		params->animationType = AnimationType::ATTACK2;
		params->delay = 1.5f;
		params->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::MOB));
		params->entityFilters.push_back(std::make_shared<MaxTargetsFilter>(1));
		return params;
	};

	// Create as many levels of this skill as needed
	auto level1Params = createParams();
	level1Params->entityProvider = std::make_shared<CircularProvider>(250.f);
	level1Params->entityHandler = std::make_shared<DamageHandler>(50.f);
	auto level1Skill = std::make_shared<AreaOfEffectSkill>(level1Params);

	auto level2Params = createParams();
	level2Params->entityProvider = std::make_shared<CircularProvider>(275.f);
	level2Params->entityHandler = std::make_shared<DamageHandler>(58.f);
	auto level2Skill = std::make_shared<AreaOfEffectSkill>(level2Params);

	auto level3Params = createParams();
	level3Params->entityProvider = std::make_shared<CircularProvider>(300.f);
	level3Params->entityHandler = std::make_shared<DamageHandler>(65.f);
	auto level3Skill = std::make_shared<AreaOfEffectSkill>(level3Params);

	SkillLevels levels = {
			level1Skill,
			level2Skill,
			level3Skill
	};

	skillComponent.addUpgradeableSkill(SkillType::SKILL2, levels);
}

void Ember::addSkill3(ECS::Entity entity, SkillComponent& skillComponent)
{
	// AOE knockback

	// For the params that should be common to all levels of this skill, put them in
	// this lambda. The upgradeable params should be handled separately (see below)
	auto createParams = [=]()
	{
		auto params = std::make_shared<AoESkillParams>();
		params->instigator = entity;
		params->soundEffect = SoundEffect::MELEE;
		params->animationType = AnimationType::ATTACK3;
		params->delay = 1.5f;
		params->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::MOB));
		return params;
	};

	// Create as many levels of this skill as needed
	auto level1Params = createParams();
	level1Params->entityProvider = std::make_shared<CircularProvider>(350.f);
	level1Params->entityHandler = std::make_shared<KnockbackHandler>(350.f, 300.f, 40.f);
	auto level1Skill = std::make_shared<AreaOfEffectSkill>(level1Params);

	auto level2Params = createParams();
	level2Params->entityProvider = std::make_shared<CircularProvider>(375.f);
	level2Params->entityHandler = std::make_shared<KnockbackHandler>(375.f, 325.f, 45.f);
	auto level2Skill = std::make_shared<AreaOfEffectSkill>(level2Params);

	auto level3Params = createParams();
	level3Params->entityProvider = std::make_shared<CircularProvider>(400.f);
	level3Params->entityHandler = std::make_shared<KnockbackHandler>(400.f, 350.f, 50.f);
	auto level3Skill = std::make_shared<AreaOfEffectSkill>(level3Params);

	SkillLevels levels = {
			level1Skill,
			level2Skill,
			level3Skill
	};

	skillComponent.addUpgradeableSkill(SkillType::SKILL3, levels);
}