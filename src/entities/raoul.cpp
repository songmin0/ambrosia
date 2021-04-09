#include "raoul.hpp"

#include "animation/animation_components.hpp"
#include "skills/skill_component.hpp"

void Raoul::initialize(ECS::Entity entity)
{
	//////////////////////////////////////////////////////////////////////////////
	// Create sprite
	ShadedMesh& resource = cacheResource("raoul_static");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, spritePath("players/raoul/raoul_static.png"), "textured");
	}
	entity.emplace<ShadedMeshRef>(resource);

	//////////////////////////////////////////////////////////////////////////////
	// Set up animations
	auto idle_anim = AnimationData("raoul_idle", spritePath("players/raoul/idle/idle"), 62);
	auto& anims = entity.emplace<AnimationsComponent>(AnimationType::IDLE, std::make_shared<AnimationData>(idle_anim));

	auto move_anim = AnimationData("raoul_move", spritePath("players/raoul/move/move"), 32);
	anims.addAnimation(AnimationType::MOVE, std::make_shared<AnimationData>(move_anim));

	auto attack1 = AnimationData("raoul_attack1", spritePath("players/raoul/attack1/attack1"), 59, 1, true, false, vec2({ 0.03f, 0.f }));
	anims.addAnimation(AnimationType::ATTACK1, std::make_shared<AnimationData>(attack1));

	auto attack2 = AnimationData("raoul_attack2", spritePath("players/raoul/attack2/attack2"), 64, 1, true, false, vec2({ 0.03f, 0.f }));
	anims.addAnimation(AnimationType::ATTACK2, std::make_shared<AnimationData>(attack2));

	auto attack3 = AnimationData("raoul_attack3", spritePath("players/raoul/attack3/attack3"), 40, 1, true, false);
	anims.addAnimation(AnimationType::ATTACK3, std::make_shared<AnimationData>(attack3));

	auto hit = AnimationData("raoul_hit", spritePath("players/raoul/hit/hit"), 49, 1, true, false);
	anims.addAnimation(AnimationType::HIT, std::make_shared<AnimationData>(hit));

	auto defeat = AnimationData("raoul_defeat", spritePath("players/raoul/defeat/defeat"), 61, 1, true, false, vec2({ 0.16f, 0.055f }));
	anims.addAnimation(AnimationType::DEFEAT, std::make_shared<AnimationData>(defeat));

	//////////////////////////////////////////////////////////////////////////////
	// Set up skills
	auto& skillComponent = entity.emplace<SkillComponent>();
	addSkill1(entity, skillComponent);
	addSkill2(entity, skillComponent);
	addSkill3(entity, skillComponent);
}

void Raoul::addSkill1(ECS::Entity entity, SkillComponent& skillComponent)
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
		params->entityFilters.push_back(std::make_shared<MaxTargetsFilter>(1));
		return params;
	};

	// Create as many levels of this skill as needed
	auto level1Params = createParams();
	level1Params->entityProvider = std::make_shared<CircularProvider>(200.f);
	level1Params->entityHandler = std::make_shared<DamageHandler>(30.f);
	auto level1Skill = std::make_shared<AreaOfEffectSkill>(level1Params);

	auto level2Params = createParams();
	level2Params->entityProvider = std::make_shared<CircularProvider>(225.f);
	level2Params->entityHandler = std::make_shared<DamageHandler>(35.f);
	auto level2Skill = std::make_shared<AreaOfEffectSkill>(level2Params);

	auto level3Params = createParams();
	level3Params->entityProvider = std::make_shared<CircularProvider>(250.f);
	level3Params->entityHandler = std::make_shared<DamageHandler>(40.f);
	auto level3Skill = std::make_shared<AreaOfEffectSkill>(level3Params);

	SkillLevels levels = {
			level1Skill,
			level2Skill,
			level3Skill
	};

	skillComponent.addUpgradeableSkill(SkillType::SKILL1, levels);
}

void Raoul::addSkill2(ECS::Entity entity, SkillComponent& skillComponent)
{
	// Strength buff for nearby players (the instigator will also be buffed)

	// For the params that should be common to all levels of this skill, put them in
	// this lambda. The upgradeable params should be handled separately (see below)
	auto createParams = [=]()
	{
		auto params = std::make_shared<AoESkillParams>();
		params->instigator = entity;
		params->soundEffect = SoundEffect::BUFF;
		params->animationType = AnimationType::ATTACK2;
		params->delay = 1.f;
		params->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::PLAYER));
		return params;
	};

	// Create as many levels of this skill as needed
	auto level1Params = createParams();
	level1Params->entityProvider = std::make_shared<CircularProvider>(300.f);
	level1Params->entityHandler = std::make_shared<BuffHandler>(StatType::STRENGTH, 0.4f, 1);
	auto level1Skill = std::make_shared<AreaOfEffectSkill>(level1Params);

	auto level2Params = createParams();
	level2Params->entityProvider = std::make_shared<CircularProvider>(325.f);
	level2Params->entityHandler = std::make_shared<BuffHandler>(StatType::STRENGTH, 0.5f, 1);
	auto level2Skill = std::make_shared<AreaOfEffectSkill>(level2Params);

	auto level3Params = createParams();
	level3Params->entityProvider = std::make_shared<CircularProvider>(350.f);
	level3Params->entityHandler = std::make_shared<BuffHandler>(StatType::STRENGTH, 0.6f, 2);
	auto level3Skill = std::make_shared<AreaOfEffectSkill>(level3Params);

	SkillLevels levels = {
			level1Skill,
			level2Skill,
			level3Skill
	};

	skillComponent.addUpgradeableSkill(SkillType::SKILL2, levels);
}

void Raoul::addSkill3(ECS::Entity entity, SkillComponent& skillComponent)
{
	// Bone throw projectile attack

	// For the params that should be common to all levels of this skill, put them in
	// this lambda. The upgradeable params should be handled separately (see below)
	auto createParams = [=]()
	{
		auto params = std::make_shared<ProjectileSkillParams>();
		params->instigator = entity;
		params->soundEffect = SoundEffect::PROJECTILE;
		params->animationType = AnimationType::ATTACK3;
		params->delay = 0.6f;
		params->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::MOB));
		params->projectileType = ProjectileType::BONE;
		return params;
	};

	// Create as many levels of this skill as needed
	auto level1Params = createParams();
	level1Params->entityHandler = std::make_shared<DamageHandler>(15.f);
	auto level1Skill = std::make_shared<ProjectileSkill>(level1Params);

	auto level2Params = createParams();
	level2Params->entityHandler = std::make_shared<DamageHandler>(20.f);
	auto level2Skill = std::make_shared<ProjectileSkill>(level2Params);

	auto level3Params = createParams();
	level3Params->entityHandler = std::make_shared<DamageHandler>(25.f);
	auto level3Skill = std::make_shared<ProjectileSkill>(level3Params);

	SkillLevels levels = {
			level1Skill,
			level2Skill,
			level3Skill
	};

	skillComponent.addUpgradeableSkill(SkillType::SKILL3, levels);
}
