#include "chia.hpp"

#include "animation/animation_components.hpp"
#include "skills/skill_component.hpp"

void Chia::initialize(ECS::Entity entity)
{
	//////////////////////////////////////////////////////////////////////////////
	// Create sprite
	ShadedMesh& resource = cacheResource("chia_static");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, spritePath("players/chia/chia_static.png"), "textured");
	}
	entity.emplace<ShadedMeshRef>(resource);

	//////////////////////////////////////////////////////////////////////////////
	// Set up animations
	auto idle_anim = AnimationData("chia_idle", spritePath("players/chia/idle/idle"), 61);
	auto& anims = entity.emplace<AnimationsComponent>(AnimationType::IDLE, std::make_shared<AnimationData>(idle_anim));

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

	//////////////////////////////////////////////////////////////////////////////
	// Set up skills
	auto& skillComponent = entity.emplace<SkillComponent>();
	addSkill1(entity, skillComponent);
	addSkill2(entity, skillComponent);
	addSkill3(entity, skillComponent);
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
	level2Params->entityHandler = std::make_shared<BuffHandler>(StatType::HP_SHIELD, 40.f, 2);
	auto level2Skill = std::make_shared<AreaOfEffectSkill>(level2Params);

	auto level3Params = createParams();
	level3Params->entityHandler = std::make_shared<BuffHandler>(StatType::HP_SHIELD, 50.f, 3);
	auto level3Skill = std::make_shared<AreaOfEffectSkill>(level3Params);

	SkillLevels levels = {
			level1Skill,
			level2Skill,
			level3Skill
	};

	skillComponent.addUpgradeableSkill(SkillType::SKILL3, levels);
}
