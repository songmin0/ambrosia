#include "raoul.hpp"

#include "rendering/render.hpp"
#include "animation/animation_components.hpp"
#include "game/turn_system.hpp"
#include "skills/skill_component.hpp"
#include "ui/ui_entities.hpp"

ECS::Entity Raoul::commonInit()
{
	auto entity = ECS::Entity();
	entity.emplace<RenderableComponent>(RenderLayer::PLAYER_AND_MOB);

	// Animations
	auto idle_anim = AnimationData("raoul_idle", spritePath("players/raoul/idle/idle"), 62);
	AnimationsComponent& anims = entity.emplace<AnimationsComponent>(AnimationType::IDLE, std::make_shared<AnimationData>(idle_anim));

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

	// Player and Turn Components
	entity.emplace<TurnSystem::TurnComponent>();
	entity.emplace<PlayerComponent>().player = PlayerType::RAOUL;

	// Initialize skills
	auto& skillComponent = entity.emplace<SkillComponent>();

	// Melee hit
	auto meleeParams = std::make_shared<AoESkillParams>();
	meleeParams->instigator = entity;
	meleeParams->soundEffect = SoundEffect::MELEE;
	meleeParams->animationType = AnimationType::ATTACK1;
	meleeParams->delay = 1.f;
	meleeParams->entityProvider = std::make_shared<CircularProvider>(200.f);
	meleeParams->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::MOB));
	meleeParams->entityFilters.push_back(std::make_shared<MaxTargetsFilter>(1));
	meleeParams->entityHandler = std::make_shared<DamageHandler>(30.f);
	skillComponent.addSkill(SkillType::SKILL1, std::make_shared<AreaOfEffectSkill>(meleeParams));

	// Strength buff for nearby players (the instigator will also be buffed)
	auto strengthBuffParams = std::make_shared<AoESkillParams>();
	strengthBuffParams->instigator = entity;
	strengthBuffParams->soundEffect = SoundEffect::BUFF;
	strengthBuffParams->animationType = AnimationType::ATTACK2;
	strengthBuffParams->delay = 1.f;
	strengthBuffParams->entityProvider = std::make_shared<CircularProvider>(300.f);
	strengthBuffParams->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::PLAYER));
	strengthBuffParams->entityHandler = std::make_shared<BuffHandler>(StatType::STRENGTH, 0.1f, 60.f);
	skillComponent.addSkill(SkillType::SKILL2, std::make_shared<AreaOfEffectSkill>(strengthBuffParams));

	// Bone throw projectile attack
	auto boneThrowParams = std::make_shared<ProjectileSkillParams>();
	boneThrowParams->instigator = entity;
	boneThrowParams->soundEffect = SoundEffect::PROJECTILE;
	boneThrowParams->animationType = AnimationType::ATTACK3;
	boneThrowParams->delay = 0.6f;
	boneThrowParams->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::MOB));
	boneThrowParams->entityHandler = std::make_shared<DamageHandler>(20.f);
	boneThrowParams->projectileType = ProjectileType::BONE;
	skillComponent.addSkill(SkillType::SKILL3, std::make_shared<ProjectileSkill>(boneThrowParams));

	entity.emplace<Raoul>();
	return entity;
};

ECS::Entity Raoul::createRaoul(json configValues)
{
	auto entity = commonInit();

	ShadedMesh& resource = cacheResource("raoul_static");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, spritePath("players/raoul/raoul_static.png"), "textured");
	}
	entity.emplace<ShadedMeshRef>(resource);

	// Setting initial motion values
	Motion& motion = entity.emplace<Motion>();
	motion.position = vec2(configValues.at("position")[0], configValues.at("position")[1]);
	motion.moveRange = 100.f;
	motion.colliderType = CollisionGroup::PLAYER;

	// hitbox scaling
	auto hitboxScale = vec2({ 0.6f, 0.9f });
	motion.boundingBox = motion.scale * hitboxScale * vec2({ resource.texture.size.x, resource.texture.size.y });

	// Initialize stats
	auto& statsComponent = entity.emplace<StatsComponent>();
	json stats = configValues.at("stats");
	statsComponent.stats[StatType::HP] = stats.at("hp");
	statsComponent.stats[StatType::MAXHP] = stats.at("hp");
	statsComponent.stats[StatType::AMBROSIA] = stats.at("ambrosia");
	statsComponent.stats[StatType::STRENGTH] = stats.at("strength");

	//Add HP bar
	statsComponent.healthBar = HPBar::createHPBar({ motion.position.x, motion.position.y - 225.0f });
	ECS::registry<HPBar>.get(statsComponent.healthBar).offset = { 0.0f,-225.0f };
	ECS::registry<HPBar>.get(statsComponent.healthBar).statsCompEntity = entity;

	return entity;
};

ECS::Entity Raoul::createRaoul(vec2 position)
{
	auto entity = commonInit();

	std::string key = "raoul_static";
	ShadedMesh& resource = cacheResource(key);
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, spritePath("players/raoul/raoul_static.png"), "textured");
	}
	entity.emplace<ShadedMeshRef>(resource);

	// Setting initial motion values
	Motion& motion = entity.emplace<Motion>();
	motion.position = position;
	motion.colliderType = CollisionGroup::PLAYER;

	// hitbox scaling
	auto hitboxScale = vec2({ 0.6f, 0.9f });
	motion.boundingBox = motion.scale * hitboxScale * vec2({ resource.texture.size.x, resource.texture.size.y });

	// Initialize stats
	auto& statsComponent = entity.emplace<StatsComponent>();
	statsComponent.stats[StatType::MAXHP] = 100.f;
	statsComponent.stats[StatType::HP] = 100.f;
	statsComponent.stats[StatType::AMBROSIA] = 0.f;
	statsComponent.stats[StatType::STRENGTH] = 1.f;

	//Add HP bar
	statsComponent.healthBar = HPBar::createHPBar({ motion.position.x, motion.position.y - 225.0f });
	ECS::registry<HPBar>.get(statsComponent.healthBar).offset = { 0.0f,-225.0f };
	ECS::registry<HPBar>.get(statsComponent.healthBar).statsCompEntity = entity;

	return entity;
};
