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

	// Melee hit
	auto meleeParams = std::make_shared<AoESkillParams>();
	meleeParams->instigator = entity;
	meleeParams->soundEffect = SoundEffect::MELEE;
	meleeParams->animationType = AnimationType::ATTACK1;
	meleeParams->delay = 1.f;
	meleeParams->entityProvider = std::make_shared<CircularProvider>(300.f);
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
	statsComponent.stats[StatType::MAXHP] = stats.at("hp");
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
	statsComponent.stats[StatType::MAXHP] = 60.f;
	statsComponent.stats[StatType::HP] = 60.f;
	statsComponent.stats[StatType::AMBROSIA] = 0.f;
	statsComponent.stats[StatType::STRENGTH] = 1.f;

	//Add HP bar
	statsComponent.healthBar = HPBar::createHPBar({ motion.position.x, motion.position.y - 225.0f });
	ECS::registry<HPBar>.get(statsComponent.healthBar).offset = { 0.0f,-225.0f };
	ECS::registry<HPBar>.get(statsComponent.healthBar).statsCompEntity = entity;

	return entity;
};