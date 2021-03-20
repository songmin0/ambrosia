#include "ai/ai.hpp"
#include "effects/effects.hpp"
#include "effects/effect_system.hpp"
#include "entities/enemies.hpp"
#include "entities/tiny_ecs.hpp"
#include "game/game_state_system.hpp"

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

	// Move the skill effects with the player
	for (auto entity : ECS::registry<Motion>.entities) {
		if (!entity.has<PlayerComponent>() && !entity.has< AISystem::MobComponent>()) {
			continue;
		}

		auto& motion = entity.get<Motion>();
		if (entity.has<BuffedFX>()) {
			entity.get<BuffedFX>().fxEntity.get<Motion>().position = motion.position;
		}
		if (entity.has<DebuffedFX>()) {
			entity.get<DebuffedFX>().fxEntity.get<Motion>().position = motion.position;
		}
		if (entity.has<HealedFX>()) {
			entity.get<HealedFX>().fxEntity.get<Motion>().position = motion.position;
		}
		if (entity.has<ShieldedFX>()) {
			entity.get<ShieldedFX>().fxEntity.get<Motion>().position = motion.position;
		}
		if (entity.has<StunnedFX>()) {
			entity.get<StunnedFX>().fxEntity.get<Motion>().position = motion.position;
		}
	}
}

void EffectSystem::onStartFX(const StartFXEvent& event) {
	auto entity = event.entity;
	auto& motion = entity.get<Motion>();

	switch (event.fxType) {
	case FXType::BUFFED:
		if (!entity.has<BuffedFX>()) {
			auto& fxEntity = entity.emplace<BuffedFX>().fxEntity; 
			fxEntity = BuffedFX::createBuffedFX(motion.position);
			fxEntity.get<SkillFX>().order = nextOrderId();
			setBuffedOffsetAndScale(entity, fxEntity);
		}
		break;
	case FXType::DEBUFFED:
		if (!entity.has<DebuffedFX>()) {
			auto& fxEntity = entity.emplace<DebuffedFX>().fxEntity;
			fxEntity = DebuffedFX::createDebuffedFX(motion.position);
			fxEntity.get<SkillFX>().order = nextOrderId();
			setBuffedOffsetAndScale(entity, fxEntity);
		}
		break;
	case FXType::HEALED:
		if (!entity.has<HealedFX>()) {
			auto& fxEntity = entity.emplace<HealedFX>().fxEntity;
			fxEntity = HealedFX::createHealedFX(motion.position);
			fxEntity.get<SkillFX>().order = nextOrderId();
			setBuffedOffsetAndScale(entity, fxEntity);
		}
		else {
			auto& fxEntity = entity.get<HealedFX>().fxEntity;
			fxEntity.get<SkillFX>().order = nextOrderId();
			fxEntity.get<AnimationsComponent>().currAnimData->currFrame = 0;
		}
	break;
	case FXType::SHIELDED:
		if (!entity.has<ShieldedFX>()) {
			auto& fxEntity = entity.emplace<ShieldedFX>().fxEntity;
			fxEntity = ShieldedFX::createShieldedFX(motion.position);
			fxEntity.get<SkillFX>().order = nextOrderId();
			setBuffedOffsetAndScale(entity, fxEntity);
		}
		break;
	case FXType::CANDY1:
		if (!entity.has<Candy1FX>()) {
			auto& fxEntity = entity.emplace<Candy1FX>().fxEntity;
			fxEntity = Candy1FX::createCandy1FX(motion.position);
			fxEntity.get<SkillFX>().order = nextOrderId();
			setCandyOffset(entity, fxEntity);
		}
		else {
			auto& fxEntity = entity.get<Candy1FX>().fxEntity;
			fxEntity.get<SkillFX>().order = nextOrderId();
			fxEntity.get<Motion>().position = motion.position;
			fxEntity.get<AnimationsComponent>().currAnimData->currFrame = 0;
		}
		break;
	case FXType::CANDY2:
		if (!entity.has<Candy2FX>()) {
			auto& fxEntity = entity.emplace<Candy2FX>().fxEntity;
			fxEntity = Candy2FX::createCandy2FX(motion.position);
			fxEntity.get<SkillFX>().order = nextOrderId();
			setCandyOffset(entity, fxEntity);
		}
		else {
			auto& fxEntity = entity.get<Candy2FX>().fxEntity;
			fxEntity.get<SkillFX>().order = nextOrderId();
			fxEntity.get<Motion>().position = motion.position;
			fxEntity.get<AnimationsComponent>().currAnimData->currFrame = 0;
		}
		break;
	case FXType::BLUEBERRIED:
		if (!entity.has<BlueberriedFX>()) {
			auto& fxEntity = entity.emplace<BlueberriedFX>().fxEntity;
			fxEntity = BlueberriedFX::createBlueberriedFX(motion.position);
			fxEntity.get<SkillFX>().order = nextOrderId();
			setBlueberriedOffset(entity, fxEntity);
		}
		else {
			auto& fxEntity = entity.get<BlueberriedFX>().fxEntity;
			fxEntity.get<SkillFX>().order = nextOrderId();
			fxEntity.get<Motion>().position = motion.position;
			fxEntity.get<AnimationsComponent>().currAnimData->currFrame = 0;
		}
		break;
	case FXType::STUNNED:
		if (!entity.has<StunnedFX>()) {
			auto& fxEntity = entity.emplace<StunnedFX>().fxEntity;
			fxEntity = StunnedFX::createStunnedFX(motion.position);
			fxEntity.get<SkillFX>().order = nextOrderId();
			setStunnedOffsetAndScale(entity, fxEntity);
		}
		break;
	default:
		break;
	}
}

void EffectSystem::onStopFX(const StopFXEvent& event) {
	auto entity = event.entity;

	switch (event.fxType) {
	case FXType::BUFFED:
		if (entity.has<BuffedFX>()) {
			ECS::ContainerInterface::removeAllComponentsOf(entity.get<BuffedFX>().fxEntity);
			entity.remove<BuffedFX>();
		}
		break;
	case FXType::DEBUFFED:
		if (entity.has<DebuffedFX>()) {
			ECS::ContainerInterface::removeAllComponentsOf(entity.get<DebuffedFX>().fxEntity);
			entity.remove<DebuffedFX>();
		}
		break;
	case FXType::HEALED:
		// Does not have a stopFX event
		break;
	case FXType::SHIELDED:
		if (entity.has<ShieldedFX>()) {
			ECS::ContainerInterface::removeAllComponentsOf(entity.get<ShieldedFX>().fxEntity);
			entity.remove<ShieldedFX>();
		}
		break;
	case FXType::CANDY1:
		// Does not have a stopFX event
		break;
	case FXType::CANDY2:
		// Does not have a stopFX event
		break;
	case FXType::BLUEBERRIED:
		// Does not have a stopFX event
		break;
	case FXType::STUNNED:
		if (entity.has<StunnedFX>()) {
			ECS::ContainerInterface::removeAllComponentsOf(entity.get<StunnedFX>().fxEntity);
			entity.remove<StunnedFX>();
		}
		break;
	default:
		break;
	}
}

// Sets offset and scale for buffed, debuffed, shielded, and healed FXs
void EffectSystem::setBuffedOffsetAndScale(ECS::Entity entity, ECS::Entity fxEntity) {
	auto& motion = fxEntity.get<Motion>();
	auto& skillFX = fxEntity.get<SkillFX>();
	if (entity.has<PlayerComponent>()) {
		skillFX.offset = vec2(0, 25);
	}
	else if (entity.has<Egg>()) {
		skillFX.offset = vec2(0, 25);
	}
	else if (entity.has<Pepper>()) {
		skillFX.offset = vec2(0, 25);
	}
	else if (entity.has<Milk>()) {
		skillFX.offset = vec2(0, 25);
	}
	else if (entity.has<Potato>()) {
		skillFX.offset = vec2(0, 25);
		motion.scale = vec2(2, 2);
	}
	else if (entity.has<MashedPotato>()) {
		skillFX.offset = vec2(0, 0);
		motion.scale = vec2(2, 2);
	}
	else if (entity.has<PotatoChunk>()) {
		skillFX.offset = vec2(0, 25);
	}
}

void EffectSystem::setStunnedOffsetAndScale(ECS::Entity entity, ECS::Entity fxEntity) {
	auto& motion = fxEntity.get<Motion>();
	auto& skillFX = fxEntity.get<SkillFX>();
	if (entity.has<PlayerComponent>()) {
		skillFX.offset = vec2(0, -50);
	}
	else if (entity.has<Egg>()) {
		skillFX.offset = vec2(0, 10);
	}
	else if (entity.has<Pepper>()) {
		skillFX.offset = vec2(0, -50);
	}
	else if (entity.has<Milk>()) {
		skillFX.offset = vec2(0, -50);
	}
	else if (entity.has<Potato>()) {
		skillFX.offset = vec2(0, 0);
		motion.scale = vec2(1.5, 1.5);
	}
	else if (entity.has<MashedPotato>()) {
		skillFX.offset = vec2(0, 0);
		motion.scale = vec2(1.5, 1.5);
	}
	else if (entity.has<PotatoChunk>()) {
		skillFX.offset = vec2(0, 25);
	}
}

void EffectSystem::setBlueberriedOffset(ECS::Entity entity, ECS::Entity fxEntity) {
	auto& skillFX = fxEntity.get<SkillFX>();
	if (entity.has<PlayerComponent>()) {
		skillFX.offset = vec2(0, -20);
	}
	else if (entity.has<Egg>()) {
		skillFX.offset = vec2(0, 0);
	}
	else if (entity.has<Pepper>()) {
		skillFX.offset = vec2(0, -20);
	}
	else if (entity.has<Milk>()) {
		skillFX.offset = vec2(0, -20);
	}
	else if (entity.has<Potato>()) {
		skillFX.offset = vec2(0, -100);
	}
	else if (entity.has<MashedPotato>()) {
		skillFX.offset = vec2(0, -80);
	}
	else if (entity.has<PotatoChunk>()) {
		skillFX.offset = vec2(0, 10);
	}
}

void EffectSystem::setCandyOffset(ECS::Entity entity, ECS::Entity fxEntity) {
	auto& skillFX = fxEntity.get<SkillFX>();
	if (entity.has<PlayerComponent>()) {
		skillFX.offset = vec2(0, 0);
	}
	else if (entity.has<Egg>()) {
		skillFX.offset = vec2(0, 30);
	}
	else if (entity.has<Pepper>()) {
		skillFX.offset = vec2(0, 0);
	}
	else if (entity.has<Milk>()) {
		skillFX.offset = vec2(0, 0);
	}
	else if (entity.has<Potato>()) {
		skillFX.offset = vec2(0, -120);
	}
	else if (entity.has<MashedPotato>()) {
		skillFX.offset = vec2(0, -100);
	}
	else if (entity.has<PotatoChunk>()) {
		skillFX.offset = vec2(0, 50);
	}
}