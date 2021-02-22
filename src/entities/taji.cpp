#include "taji.hpp"

#include "rendering/render.hpp"
#include "animation/animation_components.hpp"
#include "game/turn_system.hpp"

ECS::Entity Taji::createTaji(vec2 position, float colourShift)
{
	auto entity = ECS::Entity();

	std::string key = "taji_static";
	ShadedMesh& resource = cacheResource(key);
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, spritePath("players/taji/taji_static.png"), "textured");
	}

	entity.emplace<ShadedMeshRef>(resource);

	// Setting initial motion values
	Motion& motion = entity.emplace<Motion>();
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ 0.97f, 0.97f });
	motion.colliderType = CollisionGroup::PLAYER;
	motion.collidesWith = CollisionGroup::MOB;

	// hitbox scaling
	auto hitboxScale = vec2({ 0.6f, 0.9f });
	motion.boundingBox = motion.scale * hitboxScale * vec2({ resource.texture.size.x, resource.texture.size.y });

	// Animations
	auto idle_anim = new AnimationData(
			"taji_idle", spritePath("players/taji/idle/idle"), 62);
	AnimationsComponent& anims = entity.emplace<AnimationsComponent>(AnimationType::IDLE, *idle_anim);

	auto move_anim = new AnimationData(
			"taji_move", spritePath("players/taji/move/move"), 32);
	anims.addAnimation(AnimationType::MOVE, *move_anim);

	entity.emplace<Taji>();

	//Add the turn component
	entity.emplace<TurnSystem::TurnComponent>();

	//Add the player component
	entity.emplace<PlayerComponent>().player = PlayerType::TAJI;

	// Temporary colour shift
	entity.emplace<ColourShift>().colour = colourShift;

	// Initialize stats
	auto& statsComponent = entity.emplace<StatsComponent>();
	statsComponent.stats[StatType::HP] = 100.f;
	statsComponent.stats[StatType::AMBROSIA] = 0.f;
	statsComponent.stats[StatType::STRENGTH] = 1.f;

	// Initialize skills
	auto& skillComponent = entity.emplace<SkillComponent>();

	// Melee hit
	SkillParams meleeParams;
	meleeParams.instigator = entity;
	meleeParams.animationType = AnimationType::ATTACK1;
	meleeParams.delay = 1.f;
	meleeParams.damage = 20.f;
	meleeParams.range = 300.f;
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
	skillComponent.addSkill(SkillType::SKILL3, std::make_shared<BoneThrowSkill>(boneThrowParams));

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
