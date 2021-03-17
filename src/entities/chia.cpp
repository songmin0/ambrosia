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

	// Blueberry projectile that heals allies and damages enemies
	auto blueberryParams = std::make_shared<ProjectileSkillParams>();
	blueberryParams->instigator = entity;
	blueberryParams->soundEffect = SoundEffect::PROJECTILE;
	blueberryParams->animationType = AnimationType::ATTACK1;
	blueberryParams->delay = 0.6f;
	blueberryParams->entityFilters.push_back(std::make_shared<InstigatorFilter>(entity));
	blueberryParams->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::PLAYER | CollisionGroup::MOB));
	blueberryParams->entityHandler = std::make_shared<HealAndDamageHandler>(CollisionGroup::PLAYER, 30.f, CollisionGroup::MOB, 20.f);
	blueberryParams->entityHandler->addFX(FXType::BLUEBERRIED);
	blueberryParams->projectileType = ProjectileType::BLUEBERRY;
	skillComponent.addSkill(SkillType::SKILL1, std::make_shared<ProjectileSkill>(blueberryParams));

	// Debuff the target's strength and deal damage (need to click on the target you want to attack)
	auto debuffAndDamageParams = std::make_shared<AoESkillParams>();
	debuffAndDamageParams->instigator = entity;
	debuffAndDamageParams->soundEffect = SoundEffect::DEBUFF;
	debuffAndDamageParams->animationType = AnimationType::ATTACK2;
	debuffAndDamageParams->delay = 1.f;
	debuffAndDamageParams->entityProvider = std::make_shared<MouseClickProvider>(100.f);
	debuffAndDamageParams->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::MOB));
	debuffAndDamageParams->entityFilters.push_back(std::make_shared<MaxTargetsFilter>(1));
	debuffAndDamageParams->entityHandler = std::make_shared<DebuffAndDamageHandler>(StatType::STRENGTH, -0.1f, 60.f, 8.f);
	skillComponent.addSkill(SkillType::SKILL2, std::make_shared<AreaOfEffectSkill>(debuffAndDamageParams));

	// Grant x amount of HP shield to all allies (for 60 seconds...but change it to one turn when buffs become turn-based)
	auto hpShieldParams = std::make_shared<AoESkillParams>();
	hpShieldParams->instigator = entity;
	hpShieldParams->soundEffect = SoundEffect::BUFF; // TODO: Update this when we add an appropriate sound effect
	hpShieldParams->animationType = AnimationType::ATTACK3;
	hpShieldParams->delay = 0.6f;
	hpShieldParams->entityProvider = std::make_shared<AllEntitiesProvider>();
	hpShieldParams->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::PLAYER));
	hpShieldParams->entityHandler = std::make_shared<BuffHandler>(StatType::HP_SHIELD, 25.f, 60.f);
	skillComponent.addSkill(SkillType::SKILL3, std::make_shared<AreaOfEffectSkill>(hpShieldParams));

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
};
