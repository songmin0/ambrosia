// Header
#include "enemies.hpp"
#include "rendering/render.hpp"
#include "animation/animation_components.hpp"
#include "ai/ai.hpp"
#include "ai/behaviour_tree.hpp"
#include "game/turn_system.hpp"
#include "ui/ui_entities.hpp"
#include "ai/swarm_behaviour.hpp"

namespace
{
	StatsComponent& createStats(ECS::Entity entity, const json& stats)
	{
		auto tryLoadStat = [&](std::string name) -> float
		{
			if (stats.contains(name))
			{
				return stats[name];
			}
			return 0.f;
		};

		auto& statsComponent = entity.emplace<StatsComponent>();
		statsComponent.setBaseValue(StatType::HP, tryLoadStat("hp"));
		statsComponent.setBaseValue(StatType::MAX_HP, tryLoadStat("hp"));
		statsComponent.setBaseValue(StatType::STRENGTH, tryLoadStat("strength"));
		statsComponent.setBaseValue(StatType::AMBROSIA, tryLoadStat("ambrosia"));
		statsComponent.setBaseValue(StatType::NUM_ULT_LEFT, tryLoadStat("maxNumUlt"));
		statsComponent.setBaseValue(StatType::MAX_NUM_ULT, tryLoadStat("maxNumUlt"));

		return statsComponent;
	}
}

ECS::Entity Egg::createEgg(json stats, json position)
{
	auto entity = ECS::Entity();

	ShadedMesh& resource = cacheResource("egg_static");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, spritePath("enemies/egg/egg_static.png"), "textured");
	}
	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<RenderableComponent>(RenderLayer::PLAYER_AND_MOB);

	// Give it a Mob component
	entity.emplace<AISystem::MobComponent>();
	auto& btType = entity.emplace<BehaviourTreeType>();
	btType.mobType = MobType::EGG;

	entity.emplace<TurnSystem::TurnComponent>();

	// Setting initial motion values
	Motion& motion = entity.emplace<Motion>();
	motion.position = vec2(position[0], position[1]);
	motion.orientation = -1;
	motion.scale = vec2({ 0.8f * (float) position[2] , 0.8f});
	motion.colliderType = CollisionGroup::MOB;

	// hitbox scaling
	auto hitboxScale = vec2({ 0.65f, 0.7f });
	motion.boundingBox = motion.scale * hitboxScale * vec2({ resource.texture.size.x, resource.texture.size.y });

	// Animations
	auto idle_anim = AnimationData("egg_idle", spritePath("enemies/egg/idle/idle"), 76);
	AnimationsComponent& anims = entity.emplace<AnimationsComponent>(AnimationType::IDLE, std::make_shared<AnimationData>(idle_anim));

	auto move_anim = AnimationData("egg_move", spritePath("enemies/egg/move/move"), 51);
	anims.addAnimation(AnimationType::MOVE, std::make_shared<AnimationData>(move_anim));

	auto hit_anim = AnimationData("egg_hit", spritePath("enemies/egg/hit/hit"), 29, 1, true, false);
	anims.addAnimation(AnimationType::HIT, std::make_shared<AnimationData>(hit_anim));

	auto attack1_anim = AnimationData("egg_attack1", spritePath("enemies/egg/attack1/attack1"), 36, 1, true, false);
	anims.addAnimation(AnimationType::ATTACK1, std::make_shared<AnimationData>(attack1_anim));

	auto defeat_anim = AnimationData("egg_defeat", spritePath("enemies/egg/defeat/defeat"), 48, 1, true, false);
	anims.addAnimation(AnimationType::DEFEAT, std::make_shared<AnimationData>(defeat_anim));

	// Initialize stats
	auto& statsComponent = createStats(entity, stats);

	//Add HP bar
	statsComponent.healthBar = HPBar::createHPBar({ motion.position.x, motion.position.y - 150.0f });
	ECS::registry<HPBar>.get(statsComponent.healthBar).offset = { 0.0f,-150.0f };
	ECS::registry<HPBar>.get(statsComponent.healthBar).statsCompEntity = entity;
	ECS::registry<HPBar>.get(statsComponent.healthBar).isMob = true;

	// Initialize skills
	auto& skillComponent = entity.emplace<SkillComponent>();

	// Egg shell projectile
	auto eggShellParams = std::make_shared<ProjectileSkillParams>();
	eggShellParams->instigator = entity;
	eggShellParams->soundEffect = SoundEffect::PROJECTILE;
	eggShellParams->animationType = AnimationType::ATTACK1;
	eggShellParams->delay = 0.6f;
	eggShellParams->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::PLAYER));
	eggShellParams->entityHandler = std::make_shared<DamageHandler>(30.f);
	eggShellParams->projectileType = ProjectileType::EGG_SHELL;
	skillComponent.addSkill(SkillType::SKILL1, std::make_shared<ProjectileSkill>(eggShellParams));

	entity.emplace<Egg>();
	return entity;
};

ECS::Entity Pepper::createPepper(json stats, json position)
{
	auto entity = ECS::Entity();

	ShadedMesh& resource = cacheResource("pepper_static");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, spritePath("enemies/pepper/pepper_static.png"), "textured");
	}
	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<RenderableComponent>(RenderLayer::PLAYER_AND_MOB);

	// Give it a mob component
	entity.emplace<AISystem::MobComponent>();
	auto& btType = entity.emplace<BehaviourTreeType>();
	btType.mobType = MobType::PEPPER;

	entity.emplace<TurnSystem::TurnComponent>();

	// Setting initial motion values
	Motion& motion = entity.emplace<Motion>();
	motion.position = vec2(position[0], position[1]);
	motion.moveRange = 1500.f;
	motion.orientation = -1;
	auto hitboxScale = vec2({ 0.4f, 0.7f });
	motion.scale = vec2(0.9f * (float) position[2], 0.9f);
	motion.boundingBox = motion.scale * hitboxScale * vec2({ resource.texture.size.x, resource.texture.size.y });
	motion.colliderType = CollisionGroup::MOB;

	// Animations
	auto idle_and_run = AnimationData("pepper_idle", spritePath("enemies/pepper/idle/idle"), 74);
	AnimationsComponent& anims = entity.emplace<AnimationsComponent>(AnimationType::IDLE, std::make_shared<AnimationData>(idle_and_run));

	auto hit_anim = AnimationData("pepper_hit", spritePath("enemies/pepper/hit/hit"), 24, 1, true, false);
	anims.addAnimation(AnimationType::HIT, std::make_shared<AnimationData>(hit_anim));

	auto attack1_anim = AnimationData("pepper_attack1", spritePath("enemies/pepper/attack1/attack1"), 45, 1, true, false);
	anims.addAnimation(AnimationType::ATTACK1, std::make_shared<AnimationData>(attack1_anim));

	auto defeat_anim = AnimationData("pepper_defeat", spritePath("enemies/pepper/defeat/defeat"), 41, 1, true, false);
	anims.addAnimation(AnimationType::DEFEAT, std::make_shared<AnimationData>(defeat_anim));

	// Initialize stats
	auto& statsComponent = createStats(entity, stats);

	//Add HP bar
	statsComponent.healthBar = HPBar::createHPBar({ motion.position.x, motion.position.y - 150.0f });
	ECS::registry<HPBar>.get(statsComponent.healthBar).offset = { 0.0f, -250.0f };
	ECS::registry<HPBar>.get(statsComponent.healthBar).statsCompEntity = entity;
	ECS::registry<HPBar>.get(statsComponent.healthBar).isMob = true;

	// Initialize skills
	auto& skillComponent = entity.emplace<SkillComponent>();

	// Melee hit
	auto meleeParams = std::make_shared<AoESkillParams>();
	meleeParams->instigator = entity;
	meleeParams->soundEffect = SoundEffect::MELEE;
	meleeParams->animationType = AnimationType::ATTACK1;
	meleeParams->delay = 1.f;
	meleeParams->entityProvider = std::make_shared<CircularProvider>(200.f);
	meleeParams->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::PLAYER));
	meleeParams->entityFilters.push_back(std::make_shared<MaxTargetsFilter>(1));
	meleeParams->entityHandler = std::make_shared<DamageHandler>(40.f);
	skillComponent.addSkill(SkillType::SKILL1, std::make_shared<AreaOfEffectSkill>(meleeParams));

	entity.emplace<Pepper>();
	return entity;
};

ECS::Entity Milk::createMilk(json stats, json position)
{
	auto entity = ECS::Entity();

	ShadedMesh& resource = cacheResource("milk_static");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, spritePath("enemies/milk/idle/idle_000.png"), "textured");
	}
	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<RenderableComponent>(RenderLayer::PLAYER_AND_MOB);

	// Give it a mob component
	entity.emplace<AISystem::MobComponent>();
	auto& btType = entity.emplace<BehaviourTreeType>();
	btType.mobType = MobType::MILK;

	entity.emplace<TurnSystem::TurnComponent>();

	Motion& motion = entity.emplace<Motion>();
	motion.position = vec2(position[0], position[1]);
	motion.orientation = -1;
	motion.scale = vec2(position[2], 1.f);
	auto hitboxScale = vec2({ 0.4f, 0.7f });
	motion.boundingBox = motion.scale * hitboxScale * vec2({ resource.texture.size.x, resource.texture.size.y });
	motion.colliderType = CollisionGroup::MOB;

	// Animations
	auto idle = AnimationData("milk_idle", spritePath("enemies/milk/idle/idle"), 30);
	AnimationsComponent& anims = entity.emplace<AnimationsComponent>(AnimationType::IDLE, std::make_shared<AnimationData>(idle));

	auto move = AnimationData("milk_move", spritePath("enemies/milk/move/move"), 20);
	anims.addAnimation(AnimationType::MOVE, std::make_shared<AnimationData>(move));

	auto hit_anim = AnimationData("milk_hit", spritePath("enemies/milk/hit/hit"), 12, 1, true, false);
	anims.addAnimation(AnimationType::HIT, std::make_shared<AnimationData>(hit_anim));

	auto attack1_anim = AnimationData("milk_attack1", spritePath("enemies/milk/attack1/attack1"), 27, 1, true, false);
	anims.addAnimation(AnimationType::ATTACK1, std::make_shared<AnimationData>(attack1_anim));

	auto defeat_anim = AnimationData("milk_defeat", spritePath("enemies/milk/defeat/defeat"), 23, 1, true, false, vec2({ 0.15f, 0.f }));
	anims.addAnimation(AnimationType::DEFEAT, std::make_shared<AnimationData>(defeat_anim));

	// Initialize stats
	auto& statsComponent = createStats(entity, stats);

	//Add HP bar
	statsComponent.healthBar = HPBar::createHPBar({ motion.position.x, motion.position.y - 150.0f });
	ECS::registry<HPBar>.get(statsComponent.healthBar).offset = { 0.0f, -240.0f };
	ECS::registry<HPBar>.get(statsComponent.healthBar).statsCompEntity = entity;
	ECS::registry<HPBar>.get(statsComponent.healthBar).isMob = true;

	// Initialize skills
	auto& skillComponent = entity.emplace<SkillComponent>();

	// An orb projectile that heals the target
	auto healParams = std::make_shared<ProjectileSkillParams>();
	healParams->instigator = entity;
	healParams->soundEffect = SoundEffect::PROJECTILE;
	healParams->animationType = AnimationType::ATTACK1;
	healParams->delay = 0.3f;
	healParams->entityFilters.push_back(std::make_shared<InstigatorFilter>(entity));
	healParams->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::MOB));
	healParams->entityHandler = std::make_shared<HealHandler>(20.f);
	healParams->projectileType = ProjectileType::HEAL_ORB;
	skillComponent.addSkill(SkillType::SKILL1, std::make_shared<ProjectileSkill>(healParams));

	// Use a ranged attack if there's no ally to heal
	auto rangedAttackParams = std::make_shared<ProjectileSkillParams>();
	rangedAttackParams->instigator = entity;
	rangedAttackParams->soundEffect = SoundEffect::PROJECTILE;
	rangedAttackParams->animationType = AnimationType::ATTACK1;
	rangedAttackParams->delay = 0.3f;
	rangedAttackParams->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::PLAYER));
	rangedAttackParams->entityHandler = std::make_shared<DamageHandler>(15.f);
	rangedAttackParams->projectileType = ProjectileType::DAMAGE_ORB;
	skillComponent.addSkill(SkillType::SKILL2, std::make_shared<ProjectileSkill>(rangedAttackParams));

	entity.emplace<Milk>();
	return entity;
};

ECS::Entity Potato::createPotato(json stats, json position)
{
	auto entity = ECS::Entity();

	ShadedMesh& resource = cacheResource("potato_static");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, spritePath("enemies/potato/potato_static.png"), "textured");
	}
	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<RenderableComponent>(RenderLayer::PLAYER_AND_MOB);
	
	// Give it a mob component
	entity.emplace<AISystem::MobComponent>();
	entity.emplace<HasSwarmBehaviour>();
	auto& btType = entity.emplace<BehaviourTreeType>();
	btType.mobType = MobType::POTATO;

	entity.emplace<TurnSystem::TurnComponent>();

	// Setting initial motion values
	Motion& motion = entity.emplace<Motion>();
	motion.position = vec2(position[0], position[1]);
	motion.orientation = -1;
	motion.moveRange = 0.f;
	motion.scale = vec2(1.4f * (float) position[2], 1.4f);
	auto hitboxScale = vec2({ 0.7f, 1.f });
	motion.boundingBox = motion.scale * hitboxScale * vec2({ resource.texture.size.x, resource.texture.size.y });
	motion.colliderType = CollisionGroup::MOB;

	// Animations
	auto idle = AnimationData("potato_idle", spritePath("enemies/potato/idle/idle"), 43);
	AnimationsComponent& anims = entity.emplace<AnimationsComponent>(AnimationType::IDLE, std::make_shared<AnimationData>(idle));

	auto move = AnimationData("potato_move", spritePath("enemies/potato/move/move"), 36);
	anims.addAnimation(AnimationType::MOVE, std::make_shared<AnimationData>(move));

	auto hit_anim = AnimationData("potato_hit", spritePath("enemies/potato/hit/hit"), 16, 1, true, false);
	anims.addAnimation(AnimationType::HIT, std::make_shared<AnimationData>(hit_anim));

	auto attack1_anim = AnimationData("potato_attack1", spritePath("enemies/potato/attack1/attack1"), 30, 1, true, false, { -0.02f, 0.f });
	anims.addAnimation(AnimationType::ATTACK1, std::make_shared<AnimationData>(attack1_anim));

	auto attack2_anim = AnimationData("potato_attack2", spritePath("enemies/potato/attack2/attack2"), 41, 1, true, false, { 0.02f, 0.22f });
	anims.addAnimation(AnimationType::ATTACK2, std::make_shared<AnimationData>(attack2_anim));

	auto defeat_anim = AnimationData("potato_defeat", spritePath("enemies/potato/defeat/defeat"), 47, 1, true, false, { 0.02f, 0.22f });
	anims.addAnimation(AnimationType::DEFEAT, std::make_shared<AnimationData>(defeat_anim));

	// Initialize stats
	auto& statsComponent = createStats(entity, stats);
	entity.emplace<CCImmunityComponent>();

	//Add HP bar
	statsComponent.healthBar = HPBar::createHPBar({ motion.position.x, motion.position.y - 150.0f }, { 1.f, 0.55f });
	ECS::registry<HPBar>.get(statsComponent.healthBar).offset = { 0.0f, -380.0f };
	ECS::registry<HPBar>.get(statsComponent.healthBar).statsCompEntity = entity;
	ECS::registry<HPBar>.get(statsComponent.healthBar).isMob = true;

	// Initialize skills
	auto& skillComponent = entity.emplace<SkillComponent>();

	// Skill 1, single-target melee hit
	auto meleeParams = std::make_shared<AoESkillParams>();
	meleeParams->instigator = entity;
	meleeParams->soundEffect = SoundEffect::MELEE;
	meleeParams->animationType = AnimationType::ATTACK1;
	meleeParams->delay = 1.f;
	meleeParams->entityProvider = std::make_shared<CircularProvider>(800.f);
	meleeParams->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::PLAYER));
	meleeParams->entityFilters.push_back(std::make_shared<MaxTargetsFilter>(1));
	meleeParams->entityHandler = std::make_shared<DamageHandler>(40.f);
	skillComponent.addSkill(SkillType::SKILL1, std::make_shared<AreaOfEffectSkill>(meleeParams));

	// Skill 2, large aoe blast
	auto aoeParams = std::make_shared<AoESkillParams>();
	aoeParams->instigator = entity;
	aoeParams->soundEffect = SoundEffect::MELEE;
	aoeParams->animationType = AnimationType::ATTACK2;
	aoeParams->delay = 1.f;
	aoeParams->entityProvider = std::make_shared<CircularProvider>(800.f);
	aoeParams->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::PLAYER));
	aoeParams->entityHandler = std::make_shared<DamageHandler>(70.f);
	skillComponent.addSkill(SkillType::SKILL2, std::make_shared<AreaOfEffectSkill>(aoeParams));

	entity.emplace<Potato>();
	return entity;
};

ECS::Entity MashedPotato::createMashedPotato(vec2 pos, float initHPPercent, float orientation)
{
	auto entity = ECS::Entity();

	ShadedMesh& resource = cacheResource("mashedpotato_static");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, spritePath("enemies/mashedpotato/idle/idle_000.png"), "textured");
	}
	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<RenderableComponent>(RenderLayer::PLAYER_AND_MOB);

	entity.emplace<AISystem::MobComponent>();
	auto& btType = entity.emplace<BehaviourTreeType>();
	btType.mobType = MobType::MASHED_POTATO;

	auto& turnComponent = entity.emplace<TurnSystem::TurnComponent>();
	// Make the MashedPotato entity wait until the next turn to do its first
	// attack instead of attacking immediately after it gets spawned.
	turnComponent.hasMoved = true;
	turnComponent.hasUsedSkill = true;

	// Setting initial motion values
	Motion& motion = entity.emplace<Motion>();
	motion.position = pos;
	motion.orientation = -1;
	motion.scale = vec2(1.4f * orientation, 1.4f);
	auto hitboxScale = vec2({ 0.7f, 1.f });
	motion.boundingBox = motion.scale * hitboxScale * vec2({ resource.texture.size.x, resource.texture.size.y });
	motion.colliderType = CollisionGroup::MOB;

	// Animations
	auto idle = AnimationData("mashedpotato_idle", spritePath("enemies/mashedpotato/idle/idle"), 36);
	AnimationsComponent& anims = entity.emplace<AnimationsComponent>(AnimationType::IDLE, std::make_shared<AnimationData>(idle));
	anims.addAnimation(AnimationType::MOVE, std::make_shared<AnimationData>(idle));

	auto hit_anim = AnimationData("mashedpotato_hit", spritePath("enemies/mashedpotato/hit/hit"), 15, 2, true, false);
	anims.addAnimation(AnimationType::HIT, std::make_shared<AnimationData>(hit_anim));

	auto attack1_anim = AnimationData("mashedpotato_attack1", spritePath("enemies/mashedpotato/attack1/attack1"), 25, 1, true, false);
	anims.addAnimation(AnimationType::ATTACK1, std::make_shared<AnimationData>(attack1_anim));

	auto defeat_anim = AnimationData("mashedpotato_defeat", spritePath("enemies/mashedpotato/defeat/defeat"), 16, 2, true, false);
	anims.addAnimation(AnimationType::DEFEAT, std::make_shared<AnimationData>(defeat_anim));

	// Initialize stats
	auto& statsComponent = entity.emplace<StatsComponent>();
	// TODO: MaxHP will be a constant value < x >
	// however, Mashed Potatoes will spawn with INITIAL HP that depends on how much damage its chunks took
	// the parameter initHPPercent is a % value from 0 -> 1
	// ie. if we collectively reduced chunks to 50% HP (or defeated half the chunks), then
	// Mashed Potatoes spawns with 50% of its Max HP
	statsComponent.setBaseValue(StatType::MAX_HP, 300.f);
	statsComponent.setBaseValue(StatType::HP, 300.f * initHPPercent);
	statsComponent.setBaseValue(StatType::AMBROSIA, 13.f);
	statsComponent.setBaseValue(StatType::STRENGTH, 1.f);
	entity.emplace<CCImmunityComponent>();

	//Add HP bar
	statsComponent.healthBar = HPBar::createHPBar({ motion.position.x, motion.position.y - 150.0f }, { 1.f, 0.55f });
	ECS::registry<HPBar>.get(statsComponent.healthBar).offset = { 0.0f, -380.0f };
	ECS::registry<HPBar>.get(statsComponent.healthBar).statsCompEntity = entity;
	ECS::registry<HPBar>.get(statsComponent.healthBar).isMob = true;

	// Initialize skills
	auto& skillComponent = entity.emplace<SkillComponent>();

	// Skill 1, aoe melee hit
	auto meleeParams = std::make_shared<AoESkillParams>();
	meleeParams->instigator = entity;
	meleeParams->soundEffect = SoundEffect::MELEE;
	meleeParams->animationType = AnimationType::ATTACK1;
	meleeParams->delay = 0.3f;
	meleeParams->entityProvider = std::make_shared<CircularProvider>(800.f);
	meleeParams->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::PLAYER));
	meleeParams->entityFilters.push_back(std::make_shared<MaxTargetsFilter>(2));
	meleeParams->entityHandler = std::make_shared<DamageHandler>(30.f);
	skillComponent.addSkill(SkillType::SKILL1, std::make_shared<AreaOfEffectSkill>(meleeParams));

	entity.emplace<MashedPotato>();
	return entity;
};

ECS::Entity PotatoChunk::createPotatoChunk(vec2 pos, vec2 potato_pos, float orientation)
{
	auto entity = ECS::Entity();

	ShadedMesh& resource = cacheResource("potatochunk_static");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, spritePath("enemies/potatochunk/idle/idle_000.png"), "textured");
	}
	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<RenderableComponent>(RenderLayer::PLAYER_AND_MOB);

	// we create a dummy potato entity that only holds position, as the potato is removed when it dies
	// this dummy entity will also serve as the target for the behaviour trees
	auto potato = ECS::Entity();
	potato.emplace<Motion>();
	ECS::registry<Motion>.get(potato).position = potato_pos;
	entity.emplace<ActivePotatoChunks>(potato);

	entity.emplace<AISystem::MobComponent>();
	auto& btType = entity.emplace<BehaviourTreeType>();
	btType.mobType = MobType::POTATO_CHUNK;

	entity.emplace<TurnSystem::TurnComponent>();

	Motion& motion = entity.emplace<Motion>();
	motion.position = pos;
	motion.orientation = 1;
	motion.scale = vec2(orientation, 1.f);
	auto hitboxScale = vec2({ 0.7f, 1.f });
	motion.boundingBox = motion.scale * hitboxScale * vec2({ resource.texture.size.x, resource.texture.size.y });
	motion.colliderType = CollisionGroup::MOB;

	// Animations
	auto idle = AnimationData("potatochunk_idle", spritePath("enemies/potatochunk/idle/idle"), 26);
	AnimationsComponent& anims = entity.emplace<AnimationsComponent>(AnimationType::IDLE, std::make_shared<AnimationData>(idle));
	anims.addAnimation(AnimationType::MOVE, std::make_shared<AnimationData>(idle));

	auto hit_anim = AnimationData("potatochunk_hit", spritePath("enemies/potatochunk/hit/hit"), 15, 2, true, false);
	anims.addAnimation(AnimationType::HIT, std::make_shared<AnimationData>(hit_anim));

	auto defeat_anim = AnimationData("potatochunk_defeat", spritePath("enemies/potatochunk/defeat/defeat"), 16, 2, true, false);
	anims.addAnimation(AnimationType::DEFEAT, std::make_shared<AnimationData>(defeat_anim));

	// Initialize stats
	auto& statsComponent = entity.emplace<StatsComponent>();
	statsComponent.setBaseValue(StatType::MAX_HP, 100.f);
	statsComponent.setBaseValue(StatType::HP, 100.f);
	statsComponent.setBaseValue(StatType::AMBROSIA, 13.f);
	statsComponent.setBaseValue(StatType::STRENGTH, 1.f);
	entity.emplace<CCImmunityComponent>();

	//Add HP bar
	statsComponent.healthBar = HPBar::createHPBar({ motion.position.x, motion.position.y - 150.0f });
	ECS::registry<HPBar>.get(statsComponent.healthBar).offset = { 0.0f, -130.0f };
	ECS::registry<HPBar>.get(statsComponent.healthBar).statsCompEntity = entity;
	ECS::registry<HPBar>.get(statsComponent.healthBar).isMob = true;

	// Initialize skills
	auto& skillComponent = entity.emplace<SkillComponent>();

	// A fake Skill 1 to keep the turn system happy, but chunks don't actually use skills
	auto meleeParams = std::make_shared<AoESkillParams>();
	meleeParams->instigator = entity;
	meleeParams->soundEffect = SoundEffect::NONE;
	meleeParams->animationType = AnimationType::IDLE;
	meleeParams->delay = 0.f;
	meleeParams->entityProvider = std::make_shared<CircularProvider>(0.f);
	meleeParams->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::PLAYER));
	meleeParams->entityFilters.push_back(std::make_shared<MaxTargetsFilter>(0));
	meleeParams->entityHandler = std::make_shared<DamageHandler>(0.f);
	skillComponent.addSkill(SkillType::SKILL1, std::make_shared<AreaOfEffectSkill>(meleeParams));

	entity.emplace<PotatoChunk>();
	return entity;
};

ECS::Entity Tomato::createTomato(json stats, json position)
{
	auto entity = ECS::Entity();

	ShadedMesh& resource = cacheResource("tomato_static");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, spritePath("enemies/tomato/idle/idle_000.png"), "textured");
	}
	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<RenderableComponent>(RenderLayer::PLAYER_AND_MOB);

	// Give it a Mob component
	entity.emplace<AISystem::MobComponent>();
	auto& btType = entity.emplace<BehaviourTreeType>();
	btType.mobType = MobType::TOMATO;

	entity.emplace<TurnSystem::TurnComponent>();

	// Setting initial motion values
	Motion& motion = entity.emplace<Motion>();
	motion.position = vec2(position[0], position[1]);
	motion.orientation = -1;
	motion.scale = vec2({ 0.8f * (float)position[2] , 0.8f });
	motion.colliderType = CollisionGroup::MOB;
	motion.moveRange = 200.f;
	
	// hitbox scaling
	auto hitboxScale = vec2({ 0.65f, 0.7f });
	motion.boundingBox = motion.scale * hitboxScale * vec2({ resource.texture.size.x, resource.texture.size.y });

	// Animations
	auto idle_anim = AnimationData("tomato_idle", spritePath("enemies/tomato/idle/idle"), 24, 2);
	AnimationsComponent& anims = entity.emplace<AnimationsComponent>(AnimationType::IDLE, std::make_shared<AnimationData>(idle_anim));
	anims.addAnimation(AnimationType::MOVE, std::make_shared<AnimationData>(idle_anim));

	auto hit_anim = AnimationData("tomato_hit", spritePath("enemies/tomato/hit/hit"), 12, 2, true, false);
	anims.addAnimation(AnimationType::HIT, std::make_shared<AnimationData>(hit_anim));

	auto defeat_anim = AnimationData("tomato_defeat", spritePath("enemies/tomato/defeat/defeat"), 21, 1, true, false, vec2(-0.01f, 0.05f));
	anims.addAnimation(AnimationType::DEFEAT, std::make_shared<AnimationData>(defeat_anim));

	// Initialize stats
	auto& statsComponent = createStats(entity, stats);

	//Add HP bar
	statsComponent.healthBar = HPBar::createHPBar({ motion.position.x, motion.position.y - 150.0f });
	ECS::registry<HPBar>.get(statsComponent.healthBar).offset = { 0.0f, -150.0f };
	ECS::registry<HPBar>.get(statsComponent.healthBar).statsCompEntity = entity;
	ECS::registry<HPBar>.get(statsComponent.healthBar).isMob = true;

	// Initialize skills
	auto& skillComponent = entity.emplace<SkillComponent>();

	// Kamikaze bomb skill
	auto bombParams = std::make_shared<AoESkillParams>();
	bombParams->instigator = entity;
	bombParams->soundEffect = SoundEffect::MELEE;
	bombParams->animationType = AnimationType::ATTACK1;
	bombParams->delay = 0.2f;
	bombParams->entityProvider = std::make_shared<CircularProvider>(100.f);
	bombParams->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::ALL));
	bombParams->entityHandler = std::make_shared<DamageHandler>(100.f);
	skillComponent.addSkill(SkillType::SKILL1, std::make_shared<AreaOfEffectSkill>(bombParams));

	// A fake Skill 2 so tomato's don't explode when they're not in range
	auto meleeParams = std::make_shared<AoESkillParams>();
	meleeParams->instigator = entity;
	meleeParams->soundEffect = SoundEffect::NONE;
	meleeParams->animationType = AnimationType::IDLE;
	meleeParams->delay = 0.f;
	meleeParams->entityProvider = std::make_shared<CircularProvider>(0.f);
	meleeParams->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::NONE));
	meleeParams->entityFilters.push_back(std::make_shared<MaxTargetsFilter>(0));
	meleeParams->entityHandler = std::make_shared<DamageHandler>(0.f);
	skillComponent.addSkill(SkillType::SKILL2, std::make_shared<AreaOfEffectSkill>(meleeParams));

	entity.emplace<Tomato>();
	return entity;
};

ECS::Entity Lettuce::createLettuce(json stats, json position)
{
	auto entity = ECS::Entity();

	ShadedMesh& resource = cacheResource("lettuce_static");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, spritePath("enemies/lettuce/idle/idle_000.png"), "textured");
	}
	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<RenderableComponent>(RenderLayer::PLAYER_AND_MOB);

	entity.emplace<AISystem::MobComponent>();
	auto& btType = entity.emplace<BehaviourTreeType>();
	btType.mobType = MobType::LETTUCE;
	entity.emplace<TurnSystem::TurnComponent>();

	Motion& motion = entity.emplace<Motion>();
	motion.position = vec2(position[0], position[1]);
	motion.orientation = -1;
	motion.scale = vec2({ 1.2f * (float)position[2] , 1.2f });
	motion.colliderType = CollisionGroup::MOB;
	auto hitboxScale = vec2({ 0.8f, 0.95f });
	motion.boundingBox = motion.scale * hitboxScale * vec2({ resource.texture.size.x, resource.texture.size.y });

	// Animations
	auto idle_anim = AnimationData("lettuce_idle", spritePath("enemies/lettuce/idle/idle"), 25);
	AnimationsComponent& anims = entity.emplace<AnimationsComponent>(AnimationType::IDLE, std::make_shared<AnimationData>(idle_anim));

	auto hit_anim = AnimationData("lettuce_hit", spritePath("enemies/lettuce/hit/hit"), 12, 2, true, false);
	anims.addAnimation(AnimationType::HIT, std::make_shared<AnimationData>(hit_anim));

	auto defeat_anim = AnimationData("lettuce_defeat", spritePath("enemies/lettuce/defeat/defeat"), 13, 2, true, false);
	anims.addAnimation(AnimationType::DEFEAT, std::make_shared<AnimationData>(defeat_anim));

	auto attack1_anim = AnimationData("lettuce_attack1", spritePath("enemies/lettuce/attack1/attack1"), 22, 2, true, false, vec2(-0.03f, 0.15f));
	anims.addAnimation(AnimationType::ATTACK1, std::make_shared<AnimationData>(attack1_anim));

	auto attack2_anim = AnimationData("lettuce_attack2", spritePath("enemies/lettuce/attack2/attack2"), 24, 2, true, false, vec2(0.01f, 0.f));
	anims.addAnimation(AnimationType::ATTACK2, std::make_shared<AnimationData>(attack2_anim));

	// Initialize stats
	auto& statsComponent = createStats(entity, stats);

	//Add HP bar
	statsComponent.healthBar = HPBar::createHPBar({ motion.position.x, motion.position.y - 150.0f }, { 1.1f, 0.55f });
	ECS::registry<HPBar>.get(statsComponent.healthBar).offset = { 0.0f, -360.0f };
	ECS::registry<HPBar>.get(statsComponent.healthBar).statsCompEntity = entity;
	ECS::registry<HPBar>.get(statsComponent.healthBar).isMob = true;

	// Initialize skills
	auto& skillComponent = entity.emplace<SkillComponent>();

	// Melee AoE leaf slam
	auto leafSlamParams = std::make_shared<AoESkillParams>();
	leafSlamParams->instigator = entity;
	leafSlamParams->soundEffect = SoundEffect::MELEE;
	leafSlamParams->animationType = AnimationType::ATTACK1;
	leafSlamParams->delay = 0.8f;
	leafSlamParams->entityProvider = std::make_shared<CircularProvider>(350.f);
	leafSlamParams->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::PLAYER));
	leafSlamParams->entityHandler = std::make_shared<DamageHandler>(30.f);
	skillComponent.addSkill(SkillType::SKILL1, std::make_shared<AreaOfEffectSkill>(leafSlamParams));

	// Massive heal ("ultimate"), used when players are out of range
	auto healParams = std::make_shared<AoESkillParams>();
	healParams->instigator = entity;
	healParams->soundEffect = SoundEffect::BUFF;
	healParams->animationType = AnimationType::ATTACK2;
	healParams->delay = 1.f;
	healParams->entityProvider = std::make_shared<AllEntitiesProvider>();
	healParams->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::MOB));
	healParams->entityHandler = std::make_shared<HealHandler>(100.f);
	skillComponent.addSkill(SkillType::SKILL2, std::make_shared<AreaOfEffectSkill>(healParams));

	entity.emplace<CCImmunityComponent>();
	entity.emplace<Lettuce>();
	return entity;
};

ECS::Entity SaltnPepper::createSaltnPepper(json stats, json position)
{
	auto entity = ECS::Entity();
	ShadedMesh& resource = cacheResource("saltnpepper_static");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, spritePath("enemies/saltnpepper/idle/idle_000.png"), "textured");
	}
	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<RenderableComponent>(RenderLayer::PLAYER_AND_MOB);

	// Give it a Mob component
	entity.emplace<AISystem::MobComponent>();
	auto& btType = entity.emplace<BehaviourTreeType>();
	btType.mobType = MobType::SALTNPEPPER;

	entity.emplace<TurnSystem::TurnComponent>();

	// Setting initial motion values
	Motion& motion = entity.emplace<Motion>();
	motion.position = vec2(position[0], position[1]);
	motion.orientation = -1;
	motion.scale = vec2({(float)position[2] , 1.f });
	motion.colliderType = CollisionGroup::MOB;

	// hitbox scaling
	auto hitboxScale = vec2({ 0.7f, 0.8f });
	motion.boundingBox = motion.scale * hitboxScale * vec2({ resource.texture.size.x, resource.texture.size.y });

	// Animations
	auto idle_anim = AnimationData("saltnpepper_idle", spritePath("enemies/saltnpepper/idle/idle"), 22);
	AnimationsComponent& anims = entity.emplace<AnimationsComponent>(AnimationType::IDLE, std::make_shared<AnimationData>(idle_anim));

	auto move_anim = AnimationData("saltnpepper_move", spritePath("enemies/saltnpepper/move/move"), 15, 1, true, false);
	anims.addAnimation(AnimationType::MOVE, std::make_shared<AnimationData>(move_anim));

	auto hit_anim = AnimationData("saltnpepper_hit", spritePath("enemies/saltnpepper/hit/hit"), 9, 2, true, false);
	anims.addAnimation(AnimationType::HIT, std::make_shared<AnimationData>(hit_anim));

	auto attack1_anim = AnimationData("saltnpepper_attack1", spritePath("enemies/saltnpepper/attack1/attack1"), 15, 2, true, false);
	anims.addAnimation(AnimationType::ATTACK1, std::make_shared<AnimationData>(attack1_anim));

	auto attack2_anim = AnimationData("saltnpepper_attack2", spritePath("enemies/saltnpepper/attack2/attack2"), 15, 2, true, false);
	anims.addAnimation(AnimationType::ATTACK2, std::make_shared<AnimationData>(attack2_anim));

	auto defeat_anim = AnimationData("saltnpepper_defeat", spritePath("enemies/saltnpepper/defeat/defeat"), 10, 2, true, false, vec2(-0.15f, 0.06f));
	anims.addAnimation(AnimationType::DEFEAT, std::make_shared<AnimationData>(defeat_anim));

	// Initialize stats
	auto& statsComponent = createStats(entity, stats);

	//Add HP bar
	statsComponent.healthBar = HPBar::createHPBar({ motion.position.x, motion.position.y - 150.0f });
	ECS::registry<HPBar>.get(statsComponent.healthBar).offset = { 0.0f, -220.0f };
	ECS::registry<HPBar>.get(statsComponent.healthBar).statsCompEntity = entity;
	ECS::registry<HPBar>.get(statsComponent.healthBar).isMob = true;

	// Initialize skills
	auto& skillComponent = entity.emplace<SkillComponent>();

	// Skill 1 Pepper projectile, hurts
	auto pepperParams = std::make_shared<ProjectileSkillParams>();
	pepperParams->instigator = entity;
	pepperParams->soundEffect = SoundEffect::PROJECTILE;
	pepperParams->animationType = AnimationType::ATTACK1;
	pepperParams->delay = 0.6f;
	pepperParams->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::PLAYER));
	pepperParams->entityHandler = std::make_shared<DamageHandler>(40.f);
	pepperParams->projectileType = ProjectileType::PEPPER;
	skillComponent.addSkill(SkillType::SKILL1, std::make_shared<ProjectileSkill>(pepperParams));

	// Skill 2 Salt projectile, utility debuff
	auto saltParams = std::make_shared<ProjectileSkillParams>();
	saltParams->instigator = entity;
	saltParams->soundEffect = SoundEffect::PROJECTILE;
	saltParams->animationType = AnimationType::ATTACK2;
	saltParams->delay = 0.6f;
	saltParams->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::PLAYER));
	saltParams->entityHandler = std::make_shared<DebuffAndDamageHandler>(StatType::STRENGTH, -0.3f, 1, 20.f);
	saltParams->projectileType = ProjectileType::SALT;
	skillComponent.addSkill(SkillType::SKILL2, std::make_shared<ProjectileSkill>(saltParams));

	entity.emplace<SaltnPepper>();
	return entity;
};

ECS::Entity Chicken::createChicken(json stats, json position)
{
	auto entity = ECS::Entity();

	ShadedMesh& resource = cacheResource("chicken_static");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, spritePath("enemies/chicken/idle/idle_000.png"), "textured");
	}
	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<RenderableComponent>(RenderLayer::PLAYER_AND_MOB);

	entity.emplace<AISystem::MobComponent>();
	auto& btType = entity.emplace<BehaviourTreeType>();
	btType.mobType = MobType::CHICKEN;
	entity.emplace<TurnSystem::TurnComponent>();

	Motion& motion = entity.emplace<Motion>();
	motion.position = vec2(position[0], position[1]);
	motion.orientation = -1;
	motion.scale = vec2({ 1.3f * (float)position[2] , 1.3f });
	motion.colliderType = CollisionGroup::MOB;
	auto hitboxScale = vec2({ 0.7f, 0.7f });
	motion.boundingBox = motion.scale * hitboxScale * vec2({ resource.texture.size.x, resource.texture.size.y });

	// Animations
	auto idle_anim = AnimationData("chicken_idle", spritePath("enemies/chicken/idle/idle"), 10, 2);
	AnimationsComponent& anims = entity.emplace<AnimationsComponent>(AnimationType::IDLE, std::make_shared<AnimationData>(idle_anim));

	auto hit_anim = AnimationData("chicken_hit", spritePath("enemies/chicken/hit/hit"), 8, 2, true, false);
	anims.addAnimation(AnimationType::HIT, std::make_shared<AnimationData>(hit_anim));

	auto defeat_anim = AnimationData("chicken_defeat", spritePath("enemies/chicken/defeat/defeat"), 7, 2, true, false);
	anims.addAnimation(AnimationType::DEFEAT, std::make_shared<AnimationData>(defeat_anim));

	auto attack1_anim = AnimationData("chicken_attack1", spritePath("enemies/chicken/attack1/attack1"), 10, 3, true, false);
	anims.addAnimation(AnimationType::ATTACK1, std::make_shared<AnimationData>(attack1_anim));

	auto attack2_anim = AnimationData("chicken_attack2", spritePath("enemies/chicken/attack2/attack2"), 12, 2, true, false);
	anims.addAnimation(AnimationType::ATTACK2, std::make_shared<AnimationData>(attack2_anim));

	// Initialize stats
	auto& statsComponent = createStats(entity, stats);

	//Add HP bar
	statsComponent.healthBar = HPBar::createHPBar({ motion.position.x, motion.position.y - 150.0f }, { 1.1f, 0.55f });
	ECS::registry<HPBar>.get(statsComponent.healthBar).offset = { 0.0f, -420.0f };
	ECS::registry<HPBar>.get(statsComponent.healthBar).statsCompEntity = entity;
	ECS::registry<HPBar>.get(statsComponent.healthBar).isMob = true;

	// Initialize skills
	auto& skillComponent = entity.emplace<SkillComponent>();

	// Boomerang projectile attack
	auto drumstickParams = std::make_shared<ProjectileSkillParams>();
	drumstickParams->instigator = entity;
	drumstickParams->soundEffect = SoundEffect::PROJECTILE;
	drumstickParams->animationType = AnimationType::ATTACK1;
	drumstickParams->delay = 0.8f;
	drumstickParams->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::PLAYER));
	drumstickParams->entityHandler = std::make_shared<DamageHandler>(35.f);
	drumstickParams->projectileType = ProjectileType::DRUMSTICK;
	skillComponent.addSkill(SkillType::SKILL1, std::make_shared<ProjectileSkill>(drumstickParams));

	// Massive 100% strength buff for all allies
	auto strengthBuffParams = std::make_shared<AoESkillParams>();
	strengthBuffParams->instigator = entity;
	strengthBuffParams->soundEffect = SoundEffect::BUFF;
	strengthBuffParams->animationType = AnimationType::ATTACK2;
	strengthBuffParams->delay = 1.f;
	strengthBuffParams->entityProvider = std::make_shared<AllEntitiesProvider>();
	strengthBuffParams->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::MOB));
	strengthBuffParams->entityHandler = std::make_shared<BuffHandler>(StatType::STRENGTH, 1.f, 1);
	skillComponent.addSkill(SkillType::SKILL2, std::make_shared<AreaOfEffectSkill>(strengthBuffParams));

	entity.emplace<CCImmunityComponent>();
	entity.emplace<Chicken>();
	return entity;
};

void createEnemies(json enemies) {
	for (json enemy : enemies) {
		auto type = enemy["type"];
		if (type == "egg") {
			for (json position : enemy["positions"]) {
				Egg::createEgg(enemy["stats"], position);
			}
		}

		if (type == "pepper") {
			for (json position : enemy["positions"]) {
				Pepper::createPepper(enemy["stats"], position);
			}
		}

		if (type == "milk") {
			for (json position : enemy["positions"]) {
				Milk::createMilk(enemy["stats"], position);
			}
		}

		if (type == "potato") {
			for (json position : enemy["positions"]) {
				Potato::createPotato(enemy["stats"], position);
			}
		}

		if (type == "tomato") {
			for (json position : enemy["positions"]) {
				Tomato::createTomato(enemy["stats"], position);
			}
		}

		if (type == "lettuce") {
			for (json position : enemy["positions"]) {
				Lettuce::createLettuce(enemy["stats"], position);
			}
		}

		if (type == "saltnpepper") {
			for (json position : enemy["positions"]) {
				SaltnPepper::createSaltnPepper(enemy["stats"], position);
			}
		}

		if (type == "chicken") {
			for (json position : enemy["positions"]) {
				Chicken::createChicken(enemy["stats"], position);
			}
		}
	}

}
