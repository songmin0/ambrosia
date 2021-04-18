#include "ember.hpp"

#include "animation/animation_components.hpp"
#include "skills/skill_component.hpp"

void Ember::initialize(ECS::Entity entity)
{
	//////////////////////////////////////////////////////////////////////////////
	// Create sprite
	ShadedMesh& resource = cacheResource("ember_static");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, spritePath("players/ember/ember_static.png"), "textured");
	}
	entity.emplace<ShadedMeshRef>(resource);

	//////////////////////////////////////////////////////////////////////////////
	// Set up animations
	auto idle_anim = AnimationData("ember_idle", spritePath("players/ember/idle/idle"), 60);
	auto& anims = entity.emplace<AnimationsComponent>(AnimationType::IDLE, std::make_shared<AnimationData>(idle_anim));

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

	//////////////////////////////////////////////////////////////////////////////
	// Set up skills
	auto& skillComponent = entity.emplace<SkillComponent>();
	addSkill1(entity, skillComponent);
	addSkill2(entity, skillComponent);
	addSkill3(entity, skillComponent);
}

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
	level2Params->entityProvider = std::make_shared<CircularProvider>(330.f);
	level2Params->entityHandler = std::make_shared<DamageHandler>(30.f);
	auto level2Skill = std::make_shared<AreaOfEffectSkill>(level2Params);

	auto level3Params = createParams();
	level3Params->entityProvider = std::make_shared<CircularProvider>(360.f);
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
	level3Params->entityHandler = std::make_shared<DamageHandler>(66.f);
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
	level1Params->entityHandler = std::make_shared<KnockbackHandler>(350.f, 300.f, 25.f);
	auto level1Skill = std::make_shared<AreaOfEffectSkill>(level1Params);

	auto level2Params = createParams();
	level2Params->entityProvider = std::make_shared<CircularProvider>(385.f);
	level2Params->entityHandler = std::make_shared<KnockbackHandler>(385.f, 330.f, 30.f);
	auto level2Skill = std::make_shared<AreaOfEffectSkill>(level2Params);

	auto level3Params = createParams();
	level3Params->entityProvider = std::make_shared<CircularProvider>(420.f);
	level3Params->entityHandler = std::make_shared<KnockbackHandler>(420.f, 360.f, 35.f);
	auto level3Skill = std::make_shared<AreaOfEffectSkill>(level3Params);

	SkillLevels levels = {
			level1Skill,
			level2Skill,
			level3Skill
	};

	skillComponent.addUpgradeableSkill(SkillType::SKILL3, levels);
}
