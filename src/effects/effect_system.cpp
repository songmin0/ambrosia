#include "ai/ai.hpp"
#include "effects/effects.hpp"
#include "effects/effect_system.hpp"
#include "entities/enemies.hpp"
#include "entities/tiny_ecs.hpp"
#include "game/game_state_system.hpp"

namespace
{
	// Takes a list of FX entities for a particular FX type and checks whether
	// that type of FX is already applied to the refEntity
	std::vector<ECS::Entity>::iterator getFXEntity(std::vector<ECS::Entity>& fxEntities, ECS::Entity refEntity)
	{
		auto it = std::find_if(fxEntities.begin(), fxEntities.end(), [=](ECS::Entity fxEntity)
		{
			return fxEntity.has<SkillFXData>() && fxEntity.get<SkillFXData>().refEntity.id == refEntity.id;
		});

		return it;
	}

	void findAndRemoveFX(std::vector<ECS::Entity>& fxEntities, ECS::Entity refEntity)
	{
		auto it = getFXEntity(fxEntities, refEntity);
		if (it != fxEntities.end())
		{
			ECS::ContainerInterface::removeAllComponentsOf(*it);
		}
	}
}

EffectSystem::EffectSystem() {
	startFXListener = EventSystem<StartFXEvent>::instance().registerListener(
		std::bind(&EffectSystem::onStartFX, this, std::placeholders::_1));

	stopFXListener = EventSystem<StopFXEvent>::instance().registerListener(
		std::bind(&EffectSystem::onStopFX, this, std::placeholders::_1));
}

EffectSystem::~EffectSystem() {
	if (startFXListener.isValid()) {
		EventSystem<StartFXEvent>::instance().unregisterListener(startFXListener);
	}

	if (stopFXListener.isValid()) {
		EventSystem<StopFXEvent>::instance().unregisterListener(stopFXListener);
	}
}

void EffectSystem::step() {
	if (!GameStateSystem::instance().inGameState()) {
		return;
	}

	for (int i = ECS::registry<SkillFXData>.entities.size() - 1; i >= 0; i--)
	{
		auto fxEntity = ECS::registry<SkillFXData>.entities[i];
		auto& fxData = ECS::registry<SkillFXData>.components[i];

		// If the animation doesn't cycle, check whether it's done
		if (!fxData.doesCycle && fxEntity.has<AnimationsComponent>())
		{
			// If it's done, remove the FX entity and its components
			auto& anim = fxEntity.get<AnimationsComponent>();
			if (anim.getCurrAnim() != AnimationType::EFFECT || anim.getCurrAnimProgress() >= 1.f)
			{
				ECS::ContainerInterface::removeAllComponentsOf(fxEntity);
				continue;
			}
		}

		// Keep the FX at the same position as the reference entity
		if (fxEntity.has<Motion>() && fxData.refEntity.has<Motion>())
		{
			fxEntity.get<Motion>().position = fxData.refEntity.get<Motion>().position;
		}
	}
}

void EffectSystem::onStartFX(const StartFXEvent& event) {
	auto refEntity = event.entity;
	if (!refEntity.has<Motion>() || event.fxType == FXType::NONE)
	{
		return;
	}
	auto& refMotion = refEntity.get<Motion>();

	if (event.fxType == FXType::BUFFED)
	{
		auto& fxEntities = ECS::registry<BuffedFX>.entities;
		if (getFXEntity(fxEntities, refEntity) == fxEntities.end())
		{
			auto fxEntity = BuffedFX::createBuffedFX(refEntity, refMotion.position);
			setBuffedOffsetAndScale(fxEntity);
		}
	}
	else if (event.fxType == FXType::DEBUFFED)
	{
		auto& fxEntities = ECS::registry<DebuffedFX>.entities;
		if (getFXEntity(fxEntities, refEntity) == fxEntities.end())
		{
			auto fxEntity = DebuffedFX::createDebuffedFX(refEntity, refMotion.position);
			setBuffedOffsetAndScale(fxEntity);
		}
	}
	else if (event.fxType == FXType::HEALED)
	{
		findAndRemoveFX(ECS::registry<HealedFX>.entities, refEntity);

		auto fxEntity = HealedFX::createHealedFX(refEntity, refMotion.position);
		setBuffedOffsetAndScale(fxEntity);
	}
	else if (event.fxType == FXType::SHIELDED)
	{
		auto& fxEntities = ECS::registry<ShieldedFX>.entities;
		if (getFXEntity(fxEntities, refEntity) == fxEntities.end())
		{
			auto fxEntity = ShieldedFX::createShieldedFX(refEntity, refMotion.position);
			setBuffedOffsetAndScale(fxEntity);
		}
	}
	else if (event.fxType == FXType::CANDY1)
	{
		findAndRemoveFX(ECS::registry<Candy1FX>.entities, refEntity);

		auto fxEntity = Candy1FX::createCandy1FX(refEntity, refMotion.position);
		setCandyOffset(fxEntity);
	}
	else if (event.fxType == FXType::CANDY2)
	{
		findAndRemoveFX(ECS::registry<Candy2FX>.entities, refEntity);

		auto fxEntity = Candy2FX::createCandy2FX(refEntity, refMotion.position);
		setCandyOffset(fxEntity);
	}
	else if (event.fxType == FXType::BLUEBERRIED)
	{
		findAndRemoveFX(ECS::registry<BlueberriedFX>.entities, refEntity);

		auto fxEntity = BlueberriedFX::createBlueberriedFX(refEntity, refMotion.position);
		setBlueberriedOffset(fxEntity);
	}
	else if (event.fxType == FXType::STUNNED)
	{
		if (!refEntity.has<CCImmunityComponent>())
		{
			auto& fxEntities = ECS::registry<StunnedFX>.entities;
			if (getFXEntity(fxEntities, refEntity) == fxEntities.end())
			{
				auto fxEntity = StunnedFX::createStunnedFX(refEntity, refMotion.position);
				setStunnedOffsetAndScale(fxEntity);
			}
		}
	}
}

void EffectSystem::onStopFX(const StopFXEvent& event) {
	auto refEntity = event.entity;

	if (event.fxType == FXType::BUFFED)
	{
		findAndRemoveFX(ECS::registry<BuffedFX>.entities, refEntity);
	}
	else if (event.fxType == FXType::DEBUFFED)
	{
		findAndRemoveFX(ECS::registry<DebuffedFX>.entities, refEntity);
	}
	else if (event.fxType == FXType::HEALED)
	{
		// Does not have a stopFX event
	}
	else if (event.fxType == FXType::SHIELDED)
	{
		findAndRemoveFX(ECS::registry<ShieldedFX>.entities, refEntity);
	}
	else if (event.fxType == FXType::CANDY1)
	{
		// Does not have a stopFX event
	}
	else if (event.fxType == FXType::CANDY2)
	{
		// Does not have a stopFX event
	}
	else if (event.fxType == FXType::BLUEBERRIED)
	{
		// Does not have a stopFX event
	}
	else if (event.fxType == FXType::STUNNED)
	{
		findAndRemoveFX(ECS::registry<StunnedFX>.entities, refEntity);
	}
}

// Sets offset and scale for buffed, debuffed, shielded, and healed FXs
void EffectSystem::setBuffedOffsetAndScale(ECS::Entity fxEntity)
{
	assert(fxEntity.has<SkillFXData>());
	auto& fxData = fxEntity.get<SkillFXData>();
	auto refEntity = fxData.refEntity;

	assert(fxEntity.has<Motion>());
	auto& fxMotion = fxEntity.get<Motion>();

	if (refEntity.has<PlayerComponent>()) {
		fxData.offset = vec2(0.f, 25.f);
	}
	else if (refEntity.has<Egg>()) {
		fxData.offset = vec2(0.f, 25.f);
	}
	else if (refEntity.has<Pepper>() || refEntity.has<SaltnPepper>()) {
		fxData.offset = vec2(0.f, 25.f);
	}
	else if (refEntity.has<Milk>()) {
		fxData.offset = vec2(0.f, 25.f);
	}
	else if (refEntity.has<Potato>() || refEntity.has<Chicken>()) {
		fxData.offset = vec2(0.f, 25.f);
		fxMotion.scale = vec2(2.f, 2.f);
	}
	else if (refEntity.has<MashedPotato>() || refEntity.has<Lettuce>()) {
		fxData.offset = vec2(0.f, 0.f);
		fxMotion.scale = vec2(2.f, 2.f);
	}
	else if (refEntity.has<PotatoChunk>()) {
		fxData.offset = vec2(0.f, 25.f);
	}
}

void EffectSystem::setStunnedOffsetAndScale(ECS::Entity fxEntity)
{
	assert(fxEntity.has<SkillFXData>());
	auto& fxData = fxEntity.get<SkillFXData>();
	auto refEntity = fxData.refEntity;

	assert(fxEntity.has<Motion>());
	auto& fxMotion = fxEntity.get<Motion>();

	if (refEntity.has<PlayerComponent>()) {
		fxData.offset = vec2(0.f, -50.f);
	}
	else if (refEntity.has<Egg>()) {
		fxData.offset = vec2(0.f, 10.f);
	}
	else if (refEntity.has<Pepper>()) {
		fxData.offset = vec2(0.f, -50.f);
	}
	else if (refEntity.has<Milk>()) {
		fxData.offset = vec2(0.f, -50.f);
	}
	else if (refEntity.has<Potato>()) {
		fxData.offset = vec2(0.f, 0.f);
		fxMotion.scale = vec2(1.5f, 1.5f);
	}
	else if (refEntity.has<MashedPotato>()) {
		fxData.offset = vec2(0.f, 0.f);
		fxMotion.scale = vec2(1.5f, 1.5f);
	}
	else if (refEntity.has<PotatoChunk>()) {
		fxData.offset = vec2(0.f, 25.f);
	}
}

void EffectSystem::setBlueberriedOffset(ECS::Entity fxEntity)
{
	assert(fxEntity.has<SkillFXData>());
	auto& fxData = fxEntity.get<SkillFXData>();
	auto refEntity = fxData.refEntity;

	if (refEntity.has<PlayerComponent>()) {
		fxData.offset = vec2(0.f, -20.f);
	}
	else if (refEntity.has<Egg>()) {
		fxData.offset = vec2(0.f, 0.f);
	}
	else if (refEntity.has<Pepper>()) {
		fxData.offset = vec2(0.f, -20.f);
	}
	else if (refEntity.has<Milk>()) {
		fxData.offset = vec2(0.f, -20.f);
	}
	else if (refEntity.has<Potato>()) {
		fxData.offset = vec2(0.f, -100.f);
	}
	else if (refEntity.has<MashedPotato>()) {
		fxData.offset = vec2(0.f, -80.f);
	}
	else if (refEntity.has<PotatoChunk>()) {
		fxData.offset = vec2(0.f, 10.f);
	}
}

void EffectSystem::setCandyOffset(ECS::Entity fxEntity)
{
	assert(fxEntity.has<SkillFXData>());
	auto& fxData = fxEntity.get<SkillFXData>();
	auto refEntity = fxData.refEntity;

	if (refEntity.has<PlayerComponent>()) {
		fxData.offset = vec2(0.f, 0.f);
	}
	else if (refEntity.has<Egg>()) {
		fxData.offset = vec2(0.f, 30.f);
	}
	else if (refEntity.has<Pepper>()) {
		fxData.offset = vec2(0.f, 0.f);
	}
	else if (refEntity.has<Milk>()) {
		fxData.offset = vec2(0.f, 0.f);
	}
	else if (refEntity.has<Potato>()) {
		fxData.offset = vec2(0.f, -120.f);
	}
	else if (refEntity.has<MashedPotato>()) {
		fxData.offset = vec2(0.f, -100.f);
	}
	else if (refEntity.has<PotatoChunk>()) {
		fxData.offset = vec2(0.f, 50.f);
	}
}