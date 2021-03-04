#include "raoul.hpp"

#include "rendering/render.hpp"
#include "animation/animation_components.hpp"
#include "game/turn_system.hpp"
#include "skills/skill_component.hpp"
#include <iostream>

ECS::Entity Raoul::createRaoul(json configValues, float colourShift)
{
	auto entity = ECS::Entity();

	std::string key = "raoul_static";
	ShadedMesh& resource = cacheResource(key);
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, spritePath("players/raoul/raoul_static.png"), "textured");
	}

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	// note ShadedMeshRefs will only be rendered if there is no AnimationComponent attached to the entity
	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<RenderableComponent>(RenderLayer::PLAYER_AND_MOB);

	// Setting initial motion values
	Motion& motion = entity.emplace<Motion>();

	std::cout << configValues;
	motion.position = vec2(configValues.at("position")[0], configValues.at("position")[1]);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ 1.f, 1.f });
	motion.colliderType = CollisionGroup::PLAYER;
	motion.collidesWith = CollisionGroup::MOB;

	// hitbox scaling
	auto hitboxScale = vec2({ 0.6f, 0.9f });
	motion.boundingBox = motion.scale * hitboxScale * vec2({ resource.texture.size.x, resource.texture.size.y });


	// Animations
	auto idle_anim = new AnimationData(
			"raoul_idle", spritePath("players/raoul/idle/idle"), 62, 1, false, true
	);
	AnimationsComponent& anims = entity.emplace<AnimationsComponent>(AnimationType::IDLE, *idle_anim);

	auto move_anim = new AnimationData(
			"raoul_move", spritePath("players/raoul/move/move"), 32, 1, false, true
	);
	anims.addAnimation(AnimationType::MOVE, *move_anim);

	auto attack1 = new AnimationData(
			"raoul_attack1", spritePath("players/raoul/attack1/attack1"), 59, 1, true, false, vec2({0.03f, 0.f})
	);
	anims.addAnimation(AnimationType::ATTACK1, *attack1);

	auto attack2 = new AnimationData(
			"raoul_attack2", spritePath("players/raoul/attack2/attack2"), 64, 1, true, false, vec2({ 0.03f, 0.f })
	);
	anims.addAnimation(AnimationType::ATTACK2, *attack2);

	auto attack3 = new AnimationData(
		"raoul_attack3", spritePath("players/raoul/attack3/attack3"), 40, 1, true, false
	);
	anims.addAnimation(AnimationType::ATTACK3, *attack3);

	auto hit = new AnimationData(
			"raoul_hit", spritePath("players/raoul/hit/hit"), 49, 1, true, false
	);
	anims.addAnimation(AnimationType::HIT, *hit);

	auto defeat = new AnimationData(
			"raoul_defeat", spritePath("players/raoul/defeat/defeat"), 61, 1, true, false, vec2({ 0.16f, 0.055f })
	);
	anims.addAnimation(AnimationType::DEFEAT, *defeat);

	// start off idle
	anims.changeAnimation(AnimationType::IDLE);

	entity.emplace<Raoul>();

	//Add the turn component
	entity.emplace<TurnSystem::TurnComponent>();

	//Add the player component
	entity.emplace<PlayerComponent>().player = PlayerType::RAOUL;

	// Temporary colour shift
	entity.emplace<ColourShift>().colour = colourShift;

	// Initialize stats
	auto& statsComponent = entity.emplace<StatsComponent>();
	json stats = configValues.at("stats");
	statsComponent.stats[StatType::HP] = stats.at("hp");
	statsComponent.stats[StatType::AMBROSIA] = stats.at("ambrosia");
	statsComponent.stats[StatType::STRENGTH] = stats.at("strength");

	// Initialize skills
	auto& skillComponent = entity.emplace<SkillComponent>();

	// Melee hit
	SkillParams meleeParams;
	meleeParams.instigator = entity;
	meleeParams.animationType = AnimationType::ATTACK1;
	meleeParams.delay = 1.f;
	meleeParams.damage = 20.f;
	meleeParams.range = 200.f;
	meleeParams.collideWithMultipleEntities = false;
	meleeParams.collidesWith = CollisionGroup::MOB;
	skillComponent.addSkill(SkillType::SKILL1, std::make_shared<MeleeSkill>(meleeParams));

	// Strength buff for nearby players (the instigator will also be buffed)
	StatModifier strengthBuffModifier;
	strengthBuffModifier.statType = StatType::STRENGTH;
	strengthBuffModifier.value = 0.1f;
	strengthBuffModifier.timer = 60.f;

	SkillParams strengthBuffParams;
	strengthBuffParams.instigator = entity;
	strengthBuffParams.animationType = AnimationType::ATTACK2;
	strengthBuffParams.delay = 1.f;
	strengthBuffParams.range = 300.f;
	strengthBuffParams.ignoreInstigator = false;
	strengthBuffParams.collideWithMultipleEntities = true;
	strengthBuffParams.collidesWith = CollisionGroup::PLAYER;
	skillComponent.addSkill(SkillType::SKILL2, std::make_shared<BuffProximitySkill>(strengthBuffParams, strengthBuffModifier));

	// Bone throw projectile attack
	SkillParams boneThrowParams;
	boneThrowParams.instigator = entity;
	boneThrowParams.animationType = AnimationType::ATTACK3;
	boneThrowParams.delay = 0.6f;
	boneThrowParams.damage = 50.f;
	boneThrowParams.collidesWith = CollisionGroup::MOB;
	skillComponent.addSkill(SkillType::SKILL3, std::make_shared<ProjectileSkill>(boneThrowParams, ProjectileType::BONE));

	// Placeholder, just for fun, debuff an enemy by clicking on the enemy (doesn't matter how far away they are from the player)
	StatModifier strengthDebuffModifier;
	strengthDebuffModifier.statType = StatType::STRENGTH;
	strengthDebuffModifier.value = -0.1f;
	strengthDebuffModifier.timer = 60.f;

	SkillParams strengthDebuffParams;
	strengthDebuffParams.instigator = entity;
	strengthDebuffParams.animationType = AnimationType::ATTACK2; // TEMPORARILY SETTING TO ATTACK2 ANIMATION
	strengthDebuffParams.delay = 1.f;
	strengthDebuffParams.range = 100.f;
	strengthDebuffParams.ignoreInstigator = true;
	strengthDebuffParams.collideWithMultipleEntities = false;
	strengthDebuffParams.collidesWith = CollisionGroup::MOB;
	skillComponent.addSkill(SkillType::SKILL4, std::make_shared<BuffMouseClickSkill>(strengthDebuffParams, strengthDebuffModifier));

	return entity;
};
