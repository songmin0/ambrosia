#include "ember.hpp"

#include "rendering/render.hpp"
#include "animation/animation_components.hpp"
#include "game/turn_system.hpp"
#include "ui/ui_entities.hpp"

ECS::Entity Ember::commonInit()
{
	auto entity = ECS::Entity();
	entity.emplace<RenderableComponent>(RenderLayer::PLAYER_AND_MOB);

	// Animations
	auto idle_anim = AnimationData("ember_idle", spritePath("players/ember/idle/idle"), 60);
	AnimationsComponent& anims = entity.emplace<AnimationsComponent>(AnimationType::IDLE, std::make_shared<AnimationData>(idle_anim));

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

	// Player and Turn Component
	entity.emplace<TurnSystem::TurnComponent>();
	entity.emplace<PlayerComponent>().player = PlayerType::EMBER;

	// Initialize skills
	auto& skillComponent = entity.emplace<SkillComponent>();

	// Skill 1 Melee hit
	SkillParams meleeParams;
	meleeParams.instigator = entity;
	meleeParams.animationType = AnimationType::ATTACK1;
	meleeParams.delay = 1.f;
	meleeParams.damage = 30.f;
	meleeParams.range = 300.f;
	meleeParams.collideWithMultipleEntities = true;
	meleeParams.collidesWith = CollisionGroup::MOB;
	meleeParams.soundEffect = SoundEffect::MELEE;
	skillComponent.addSkill(SkillType::SKILL1, std::make_shared<MeleeSkill>(meleeParams));

	// Skill 2 Melee hit
	SkillParams melee2Params;
	melee2Params.instigator = entity;
	melee2Params.animationType = AnimationType::ATTACK2;
	melee2Params.delay = 1.5f;
	melee2Params.damage = 60.f;
	melee2Params.range = 250.f;
	melee2Params.collideWithMultipleEntities = false;
	melee2Params.collidesWith = CollisionGroup::MOB;
	melee2Params.soundEffect = SoundEffect::MELEE;
	skillComponent.addSkill(SkillType::SKILL2, std::make_shared<MeleeSkill>(melee2Params));

	// Skill 3 AOE Knockback, without the knockback
	SkillParams melee3Params;
	melee3Params.instigator = entity;
	melee3Params.animationType = AnimationType::ATTACK3;
	melee3Params.delay = 1.5f;
	melee3Params.damage = 40.f;
	melee3Params.range = 200.f;
	melee3Params.collideWithMultipleEntities = true;
	melee3Params.collidesWith = CollisionGroup::MOB;
	melee3Params.soundEffect = SoundEffect::MELEE;
	skillComponent.addSkill(SkillType::SKILL3, std::make_shared<MeleeSkill>(melee3Params));

	entity.emplace<Ember>();
	return entity;
};

ECS::Entity Ember::createEmber(json configValues)
{
	auto entity = commonInit();

	ShadedMesh& resource = cacheResource("ember_static");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, spritePath("players/ember/ember_static.png"), "textured");
	}
	entity.emplace<ShadedMeshRef>(resource);

	// Setting initial motion values
	Motion& motion = entity.emplace<Motion>();
	motion.position = vec2(configValues.at("position")[0], configValues.at("position")[1]);
	motion.colliderType = CollisionGroup::PLAYER;
	motion.collidesWith = CollisionGroup::MOB;

	// hitbox scaling
	auto hitboxScale = vec2({ 0.4f, 0.6f });
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

ECS::Entity Ember::createEmber(vec2 position)
{
	auto entity = commonInit();

	ShadedMesh& resource = cacheResource("ember_static");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, spritePath("players/ember/ember_static.png"), "textured");
	}
	entity.emplace<ShadedMeshRef>(resource);

	// Setting initial motion values
	Motion& motion = entity.emplace<Motion>();
	motion.position = position;
	motion.colliderType = CollisionGroup::PLAYER;
	motion.collidesWith = CollisionGroup::MOB;

	// hitbox scaling
	auto hitboxScale = vec2({ 0.4f, 0.6f });
	motion.boundingBox = motion.scale * hitboxScale * vec2({ resource.texture.size.x, resource.texture.size.y });

	// Initialize stats
	auto& statsComponent = entity.emplace<StatsComponent>();
	statsComponent.stats[StatType::MAXHP] = 80.f;
	statsComponent.stats[StatType::HP] = 80.f;
	statsComponent.stats[StatType::AMBROSIA] = 0.f;
	statsComponent.stats[StatType::STRENGTH] = 1.f;

	//Add HP bar
	statsComponent.healthBar = HPBar::createHPBar({ motion.position.x, motion.position.y - 225.0f });
	ECS::registry<HPBar>.get(statsComponent.healthBar).offset = { 0.0f,-225.0f };
	ECS::registry<HPBar>.get(statsComponent.healthBar).statsCompEntity = entity;

	return entity;
};