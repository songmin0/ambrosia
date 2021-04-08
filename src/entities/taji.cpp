#include "taji.hpp"

#include "animation/animation_components.hpp"
#include "skills/skill_component.hpp"

void Taji::initialize(ECS::Entity entity)
{
	//////////////////////////////////////////////////////////////////////////////
	// Create sprite
	ShadedMesh& resource = cacheResource("taji_static");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, spritePath("players/taji/taji_static.png"), "textured");
	}
	entity.emplace<ShadedMeshRef>(resource);

	//////////////////////////////////////////////////////////////////////////////
	// Set up animations
	auto idle_anim = AnimationData("taji_idle", spritePath("players/taji/idle/idle"), 62);
	auto& anims = entity.emplace<AnimationsComponent>(AnimationType::IDLE, std::make_shared<AnimationData>(idle_anim));

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

	//////////////////////////////////////////////////////////////////////////////
	// Set up skills
	auto& skillComponent = entity.emplace<SkillComponent>();

	// Ranged damage with small area of effect
	auto aoeParams = std::make_shared<AoESkillParams>();
	aoeParams->instigator = entity;
	aoeParams->soundEffect = SoundEffect::BUFF;
	aoeParams->animationType = AnimationType::ATTACK1;
	aoeParams->delay = 1.f;
	aoeParams->entityProvider = std::make_shared<MouseClickProvider>(250.f);
	aoeParams->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::MOB));
	aoeParams->entityHandler = std::make_shared<DamageHandler>(20.f);
	aoeParams->entityHandler->addFX(FXType::CANDY1);
	skillComponent.addSkill(SkillType::SKILL1, std::make_shared<AreaOfEffectSkill>(aoeParams));

	// Single-target ranged attack, deals damage and makes the target skip a turn (that part is not supported yet)
	auto castAttackParams = std::make_shared<AoESkillParams>();
	castAttackParams->instigator = entity;
	castAttackParams->soundEffect = SoundEffect::BUFF;
	castAttackParams->animationType = AnimationType::ATTACK2;
	castAttackParams->delay = 1.f;
	castAttackParams->entityProvider = std::make_shared<MouseClickProvider>(100.f);
	castAttackParams->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::MOB));
	castAttackParams->entityFilters.push_back(std::make_shared<MaxTargetsFilter>(1));
	castAttackParams->entityHandler = std::make_shared<DebuffAndDamageHandler>(StatType::STUNNED, 1.f, 1, 20.f);
	castAttackParams->entityHandler->addFX(FXType::CANDY2);
	skillComponent.addSkill(SkillType::SKILL2, std::make_shared<AreaOfEffectSkill>(castAttackParams));

	// Small damage to all enemies, small heal to all allies
	auto healAndDamageParams = std::make_shared<AoESkillParams>();
	healAndDamageParams->instigator = entity;
	healAndDamageParams->soundEffect = SoundEffect::BUFF;
	healAndDamageParams->animationType = AnimationType::ATTACK3;
	healAndDamageParams->delay = 1.f;
	healAndDamageParams->entityProvider = std::make_shared<AllEntitiesProvider>();
	healAndDamageParams->entityFilters.push_back(std::make_shared<CollisionFilter>(CollisionGroup::PLAYER | CollisionGroup::MOB));
	healAndDamageParams->entityHandler = std::make_shared<HealAndDamageHandler>(CollisionGroup::PLAYER, 8.f, CollisionGroup::MOB, 8.f);
	skillComponent.addSkill(SkillType::SKILL3, std::make_shared<AreaOfEffectSkill>(healAndDamageParams));
}
