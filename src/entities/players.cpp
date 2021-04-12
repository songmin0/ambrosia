#include "players.hpp"
#include "raoul.hpp"
#include "taji.hpp"
#include "ember.hpp"
#include "chia.hpp"

#include "rendering/render.hpp"
#include "game/turn_system.hpp"
#include "ui/ui_entities.hpp"

namespace
{
	void createHPBar(ECS::Entity entity)
	{
		assert(entity.has<Motion>());
		auto& motion = entity.get<Motion>();

		assert(entity.has<StatsComponent>());
		auto& statsComponent = entity.get<StatsComponent>();

		statsComponent.healthBar = HPBar::createHPBar({motion.position.x, motion.position.y - 225.0f});
		ECS::registry<HPBar>.get(statsComponent.healthBar).offset = {0.0f,-225.0f};
		ECS::registry<HPBar>.get(statsComponent.healthBar).statsCompEntity = entity;
	}

	void createMotion(PlayerType type, ECS::Entity entity, const json& configValues)
	{
		// Remove existing Motion if one exists
		entity.remove<Motion>();

		// Create new Motion
		auto& motion = entity.emplace<Motion>();
		motion.position = vec2(configValues.at("position")[0],
													 configValues.at("position")[1]);
		motion.colliderType = CollisionGroup::PLAYER;

		vec2 scale;
		vec2 hitboxScale;

		if (type == PlayerType::RAOUL)
		{
			scale = Raoul::SCALE;
			hitboxScale = Raoul::HITBOX_SCALE;
		}
		else if (type == PlayerType::TAJI)
		{
			scale = Taji::SCALE;
			hitboxScale = Taji::HITBOX_SCALE;
		}
		else if (type == PlayerType::EMBER)
		{
			scale = Ember::SCALE;
			hitboxScale = Ember::HITBOX_SCALE;
		}
		else
		{
			scale = Chia::SCALE;
			hitboxScale = Chia::HITBOX_SCALE;
		}

		ShadedMesh* texmesh = entity.get<ShadedMeshRef>().reference_to_cache;
		assert(texmesh);

		motion.scale = scale;
		motion.boundingBox = motion.scale * hitboxScale * vec2(texmesh->texture.size.x,
																													 texmesh->texture.size.y);
	}
}

ECS::Entity Player::create(PlayerType type, const json& configValues)
{
	auto entity = ECS::Entity();

	entity.emplace<TurnSystem::TurnComponent>();
	entity.emplace<PlayerComponent>().player = type;

	if (type == PlayerType::RAOUL)
	{
		Raoul::initialize(entity);
	}
	else if (type == PlayerType::TAJI)
	{
		Taji::initialize(entity);
	}
	else if (type == PlayerType::EMBER)
	{
		Ember::initialize(entity);
	}
	else
	{
		Chia::initialize(entity);
	}

	// Stats
	auto& statsComponent = entity.emplace<StatsComponent>();
	json stats = configValues.at("stats");
	statsComponent.setBaseValue(StatType::HP, stats.at("hp"));
	statsComponent.setBaseValue(StatType::MAX_HP, stats.at("hp"));
	statsComponent.setBaseValue(StatType::BASE_HP, stats.at("hp"));
	statsComponent.setBaseValue(StatType::LEVEL, 1);
	statsComponent.setBaseValue(StatType::STRENGTH, stats.at("strength"));
	statsComponent.setBaseValue(StatType::BASE_STR, stats.at("strength"));

	prepareForNextMap(entity, configValues);

	return entity;
}

void Player::prepareForNextMap(ECS::Entity entity, const json& configValues)
{
	assert(entity.has<PlayerComponent>());
	PlayerType type = entity.get<PlayerComponent>().player;

	// The following tasks are appropriate to handle here, and the others are
	// triggered using the event below
	entity.remove<DeathTimer>();
	createMotion(type, entity, configValues);
	createHPBar(entity);
	enableRendering(entity);

	EventSystem<PrepForNextMapEvent>::instance().sendEvent({entity});
}

void Player::enableRendering(ECS::Entity entity)
{
	if (!entity.has<RenderableComponent>())
	{
		entity.emplace<RenderableComponent>(RenderLayer::PLAYER_AND_MOB);
	}
}

void Player::disableRendering(ECS::Entity entity)
{
	entity.remove<RenderableComponent>();
}
