#include "ember.hpp"

#include "animation/animation_components.hpp"
#include "skills/skill_component.hpp"

void Ember::initialize(ECS::Entity entity)
{
	//////////////////////////////////////////////////////////////////////////////
	// Create sprite
	ShadedMesh& resource = cacheResource("ember_static");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, spritePath("players/ember/ember_static.png"), "textured");
	}
	entity.emplace<ShadedMeshRef>(resource);

	//////////////////////////////////////////////////////////////////////////////
	// Set up animations
	auto idle_anim = AnimationData("ember_idle", spritePath("players/ember/idle/idle"), 60);
	auto& anims = entity.emplace<AnimationsComponent>(AnimationType::IDLE, std::make_shared<AnimationData>(idle_anim));

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

	//////////////////////////////////////////////////////////////////////////////
	// Set up skills
	auto& skillComponent = entity.emplace<SkillComponent>();

	// Skill 1 Melee hit
	auto meleeParams = std::make_shared<AoESkillParams>();
	meleeParams->instigator = entity;
	meleeParams->soundEffect = SoundEffect::MELEE;
	meleeParams->animationType = AnimationType::ATTACK1;
	meleeParams->delay = 1.f;
	meleeParams->entityProvider = std::make_shared<CircularProvider>(300.f);
	meleeParams->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::MOB));
	meleeParams->entityHandler = std::make_shared<DamageHandler>(25.f);
	skillComponent.addSkill(SkillType::SKILL1, std::make_shared<AreaOfEffectSkill>(meleeParams));

	// Skill 2 Melee hit
	auto melee2Params = std::make_shared<AoESkillParams>();
	melee2Params->instigator = entity;
	melee2Params->soundEffect = SoundEffect::MELEE;
	melee2Params->animationType = AnimationType::ATTACK2;
	melee2Params->delay = 1.5f;
	melee2Params->entityProvider = std::make_shared<CircularProvider>(250.f);
	melee2Params->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::MOB));
	melee2Params->entityFilters.push_back(std::make_shared<MaxTargetsFilter>(1));
	melee2Params->entityHandler = std::make_shared<DamageHandler>(50.f);
	skillComponent.addSkill(SkillType::SKILL2, std::make_shared<AreaOfEffectSkill>(melee2Params));

	// Skill 3 AOE Knockback, without the knockback
	auto melee3Params = std::make_shared<AoESkillParams>();
	melee3Params->instigator = entity;
	melee3Params->soundEffect = SoundEffect::MELEE;
	melee3Params->animationType = AnimationType::ATTACK3;
	melee3Params->delay = 1.5f;
	melee3Params->entityProvider = std::make_shared<CircularProvider>(350.f);
	melee3Params->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::MOB));
	melee3Params->entityHandler = std::make_shared<KnockbackHandler>(350.f, 300.f, 40.f);
	skillComponent.addSkill(SkillType::SKILL3, std::make_shared<AreaOfEffectSkill>(melee3Params));
}
