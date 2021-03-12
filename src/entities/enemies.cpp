// Header
#include "enemies.hpp"

#include "rendering/render.hpp"
#include "animation/animation_components.hpp"
#include "ai/ai.hpp"
#include "ai/behaviour_tree.hpp"
#include "game/turn_system.hpp"
#include "ui/ui_entities.hpp"

ECS::Entity Egg::createEgg(vec2 pos)
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
	// Set appropriate Behaviour Tree
	//entity.emplace<EggBehaviourTree>();
	//EggBehaviourTree bt;

	auto& btType = entity.emplace<BehaviourTreeType>();
	btType.mobType = MobType::EGG;

	entity.emplace<TurnSystem::TurnComponent>();

	// Setting initial motion values
	Motion& motion = entity.emplace<Motion>();
	motion.position = pos;
	motion.scale = vec2({ 0.8f, 0.8f });
	motion.orientation = -1;
	motion.colliderType = CollisionGroup::MOB;
	motion.collidesWith = CollisionGroup::PLAYER;


	// hitbox scaling
	auto hitboxScale = vec2({ 0.65f, 0.7f });
	motion.boundingBox = motion.scale * hitboxScale * vec2({ resource.texture.size.x, resource.texture.size.y });

	// Animations
	auto idle_anim = new AnimationData(
		"egg_idle", spritePath("enemies/egg/idle/idle"), 76);
	AnimationsComponent& anims = entity.emplace<AnimationsComponent>(AnimationType::IDLE, *idle_anim);

	auto move_anim = new AnimationData(
		"egg_move", spritePath("enemies/egg/move/move"), 51);
	anims.addAnimation(AnimationType::MOVE, *move_anim);

	auto hit_anim = new AnimationData("egg_hit", spritePath("enemies/egg/hit/hit"), 29, 1, true, false);
	anims.addAnimation(AnimationType::HIT, *hit_anim);

	auto attack1_anim = new AnimationData("egg_attack1", spritePath("enemies/egg/attack1/attack1"), 36, 1, true, false);
	anims.addAnimation(AnimationType::ATTACK1, *attack1_anim);

	auto defeat_anim = new AnimationData("egg_defeat", spritePath("enemies/egg/defeat/defeat"), 48, 1, true, false);
	anims.addAnimation(AnimationType::DEFEAT, *defeat_anim);

	// Initialize stats
	auto& statsComponent = entity.emplace<StatsComponent>();
	statsComponent.stats[StatType::MAXHP] = 50.f;
	statsComponent.stats[StatType::HP] = 50.f;
	statsComponent.stats[StatType::AMBROSIA] = 0.f;
	statsComponent.stats[StatType::STRENGTH] = 1.f;

	//Add HP bar
	statsComponent.healthBar = HPBar::createHPBar({ motion.position.x, motion.position.y - 150.0f });
	ECS::registry<HPBar>.get(statsComponent.healthBar).offset = { 0.0f,-150.0f };
	ECS::registry<HPBar>.get(statsComponent.healthBar).statsCompEntity = entity;
	ECS::registry<HPBar>.get(statsComponent.healthBar).isMob = true;

	// Initialize skills
	auto& skillComponent = entity.emplace<SkillComponent>();

	// Egg shell projectile
	SkillParams eggShellParams;
	eggShellParams.instigator = entity;
	eggShellParams.animationType = AnimationType::ATTACK1;
	eggShellParams.delay = 0.6f;
	eggShellParams.damage = 30.f;
	eggShellParams.collidesWith = CollisionGroup::PLAYER;
	eggShellParams.soundEffect = SoundEffect::PROJECTILE;
	skillComponent.addSkill(SkillType::SKILL1, std::make_shared<ProjectileSkill>(eggShellParams, ProjectileType::EGG_SHELL));

	entity.emplace<Egg>();
	return entity;
};

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
	statsComponent.stats[StatType::HP] = 40.f;
	statsComponent.stats[StatType::MAXHP] = 40.f;
	statsComponent.stats[StatType::AMBROSIA] = 0.f;
	statsComponent.stats[StatType::STRENGTH] = 1.f;

	//Add HP bar
	statsComponent.healthBar = HPBar::createHPBar({ motion.position.x, motion.position.y - 150.0f });
	ECS::registry<HPBar>.get(statsComponent.healthBar).offset = { 0.0f, -250.0f };
	ECS::registry<HPBar>.get(statsComponent.healthBar).statsCompEntity = entity;
	ECS::registry<HPBar>.get(statsComponent.healthBar).isMob = true;

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

	// Initialize stats
	auto& statsComponent = entity.emplace<StatsComponent>();
	statsComponent.stats[StatType::MAXHP] = 200.f;
	statsComponent.stats[StatType::HP] = 200.f;
	statsComponent.stats[StatType::AMBROSIA] = 0.f;
	statsComponent.stats[StatType::STRENGTH] = 1.f;

	//Add HP bar
	statsComponent.healthBar = HPBar::createHPBar({ motion.position.x, motion.position.y - 150.0f }, { 1.f, 0.55f });
	ECS::registry<HPBar>.get(statsComponent.healthBar).offset = { 0.0f, -380.0f };
	ECS::registry<HPBar>.get(statsComponent.healthBar).statsCompEntity = entity;
	ECS::registry<HPBar>.get(statsComponent.healthBar).isMob = true;

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
	aoeParams.damage = 70.f;
	aoeParams.range = 800.f;
	aoeParams.collideWithMultipleEntities = true;
	aoeParams.collidesWith = CollisionGroup::PLAYER;
	aoeParams.soundEffect = SoundEffect::MELEE;
	skillComponent.addSkill(SkillType::SKILL2, std::make_shared<MeleeSkill>(aoeParams));

	entity.emplace<Potato>();
	return entity;
};

ECS::Entity MashedPotato::createMashedPotato(vec2 pos, float initHPPercent)
{
	auto entity = ECS::Entity();

	ShadedMesh& resource = cacheResource("mashedpotato_static");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, spritePath("enemies/mashedpotato/idle/idle_000.png"), "textured");
	}
	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<RenderableComponent>(RenderLayer::PLAYER_AND_MOB);

	// TODO: Figure this out for MashedPotato
	entity.emplace<AISystem::MobComponent>();
	auto& btType = entity.emplace<BehaviourTreeType>();
	btType.mobType = MobType::EGG;

	entity.emplace<TurnSystem::TurnComponent>();

	// Setting initial motion values
	Motion& motion = entity.emplace<Motion>();
	motion.position = pos;
	motion.scale = vec2(1.4f);
	motion.orientation = -1;
	auto hitboxScale = vec2({ 0.7f, 1.f });
	motion.boundingBox = motion.scale * hitboxScale * vec2({ resource.texture.size.x, resource.texture.size.y });
	motion.colliderType = CollisionGroup::MOB;
	motion.collidesWith = CollisionGroup::PLAYER;

	// Animations
	auto idle = new AnimationData("mashedpotato_idle", spritePath("enemies/mashedpotato/idle/idle"), 36);
	AnimationsComponent& anims = entity.emplace<AnimationsComponent>(AnimationType::IDLE, *idle);
	anims.addAnimation(AnimationType::MOVE, *idle);

	auto hit_anim = new AnimationData("mashedpotato_hit", spritePath("enemies/mashedpotato/hit/hit"), 15, 1, true, false);
	anims.addAnimation(AnimationType::HIT, *hit_anim);

	auto attack1_anim = new AnimationData("mashedpotato_attack1", spritePath("enemies/mashedpotato/attack1/attack1"), 25, 1, true, false);
	anims.addAnimation(AnimationType::ATTACK1, *attack1_anim);

	auto defeat_anim = new AnimationData("mashedpotato_defeat", spritePath("enemies/mashedpotato/defeat/defeat"), 16, 1, true, false);
	anims.addAnimation(AnimationType::DEFEAT, *defeat_anim);

	// Initialize stats
	auto& statsComponent = entity.emplace<StatsComponent>();
	// TODO: MaxHP will be a constant value < x >
	// however, Mashed Potatoes will spawn with INITIAL HP that depends on how much damage its chunks took
	// the parameter initHPPercent is a % value from 0 -> 1
	// ie. if we collectively reduced chunks to 50% HP (or defeated half the chunks), then
	// Mashed Potatoes spawns with 50% of its Max HP
	statsComponent.stats[StatType::MAXHP] = 180.f;
	statsComponent.stats[StatType::HP] = 180.f * initHPPercent;
	statsComponent.stats[StatType::AMBROSIA] = 0.f;
	statsComponent.stats[StatType::STRENGTH] = 1.f;

	//Add HP bar
	statsComponent.healthBar = HPBar::createHPBar({ motion.position.x, motion.position.y - 150.0f }, { 1.f, 0.55f });
	ECS::registry<HPBar>.get(statsComponent.healthBar).offset = { 0.0f, -380.0f };
	ECS::registry<HPBar>.get(statsComponent.healthBar).statsCompEntity = entity;
	ECS::registry<HPBar>.get(statsComponent.healthBar).isMob = true;

	// Initialize skills
	auto& skillComponent = entity.emplace<SkillComponent>();

	// Skill 1, aoe melee hit
	SkillParams meleeParams;
	meleeParams.instigator = entity;
	meleeParams.animationType = AnimationType::ATTACK1;
	meleeParams.delay = 0.3f;
	meleeParams.damage = 40.f;
	meleeParams.range = 400.f;
	meleeParams.collideWithMultipleEntities = true;
	meleeParams.collidesWith = CollisionGroup::PLAYER;
	meleeParams.soundEffect = SoundEffect::MELEE;
	skillComponent.addSkill(SkillType::SKILL1, std::make_shared<MeleeSkill>(meleeParams));

	entity.emplace<MashedPotato>();
	return entity;
};

ECS::Entity PotatoChunk::createPotatoChunk(vec2 pos, float orientation)
{
	auto entity = ECS::Entity();

	ShadedMesh& resource = cacheResource("potatochunk_static");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, spritePath("enemies/potatochunk/idle/idle_000.png"), "textured");
	}
	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<RenderableComponent>(RenderLayer::PLAYER_AND_MOB);

	// TODO: Swarm behaviour
	// ~6 chunks should be spawned in a well-spaced pattern within a certain range of the defeated potato boss
	// during their turn, the chunks should try to move together to reform into mashed potato
	entity.emplace<AISystem::MobComponent>();
	auto& btType = entity.emplace<BehaviourTreeType>();
	btType.mobType = MobType::EGG;

	entity.emplace<TurnSystem::TurnComponent>();

	Motion& motion = entity.emplace<Motion>();
	motion.position = pos;
	motion.orientation = orientation; // which way the chunk faces when spawned
	auto hitboxScale = vec2({ 0.7f, 1.f });
	motion.boundingBox = motion.scale * hitboxScale * vec2({ resource.texture.size.x, resource.texture.size.y });
	motion.colliderType = CollisionGroup::MOB;
	motion.collidesWith = CollisionGroup::PLAYER;

	// Animations
	auto idle = new AnimationData("potatochunk_idle", spritePath("enemies/potatochunk/idle/idle"), 26);
	AnimationsComponent& anims = entity.emplace<AnimationsComponent>(AnimationType::IDLE, *idle);
	anims.addAnimation(AnimationType::MOVE, *idle);

	auto hit_anim = new AnimationData("potatochunk_hit", spritePath("enemies/potatochunk/hit/hit"), 15, 1, true, false);
	anims.addAnimation(AnimationType::HIT, *hit_anim);

	auto defeat_anim = new AnimationData("potatochunk_defeat", spritePath("enemies/potatochunk/defeat/defeat"), 16, 1, true, false);
	anims.addAnimation(AnimationType::DEFEAT, *defeat_anim);

	// Initialize stats
	auto& statsComponent = entity.emplace<StatsComponent>();
	statsComponent.stats[StatType::MAXHP] = 30.f;
	statsComponent.stats[StatType::HP] = 30.f;
	statsComponent.stats[StatType::AMBROSIA] = 0.f;
	statsComponent.stats[StatType::STRENGTH] = 1.f;

	//Add HP bar
	statsComponent.healthBar = HPBar::createHPBar({ motion.position.x, motion.position.y - 150.0f });
	ECS::registry<HPBar>.get(statsComponent.healthBar).offset = { 0.0f, -130.0f };
	ECS::registry<HPBar>.get(statsComponent.healthBar).statsCompEntity = entity;
	ECS::registry<HPBar>.get(statsComponent.healthBar).isMob = true;

	// Initialize skills
	auto& skillComponent = entity.emplace<SkillComponent>();

	// A fake Skill 1 to keep the turn system happy, but chunks don't actually use skills
	SkillParams meleeParams;
	meleeParams.instigator = entity;
	meleeParams.animationType = AnimationType::IDLE;
	meleeParams.delay = 0.f;
	meleeParams.damage = 0.f;
	meleeParams.range = 0.f;
	meleeParams.collideWithMultipleEntities = false;
	meleeParams.collidesWith = CollisionGroup::PLAYER;
	meleeParams.soundEffect = SoundEffect::MELEE;
	skillComponent.addSkill(SkillType::SKILL1, std::make_shared<MeleeSkill>(meleeParams));

	entity.emplace<PotatoChunk>();
	return entity;
};

