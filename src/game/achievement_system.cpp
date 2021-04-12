#include "achievement_system.hpp"
#include "stats_component.hpp"
#include "rendering/text.hpp"

AchievementSystem::AchievementSystem()
{
	//assert(sizeof(Achievement::SIZE_OF_ENUM) == sizeof(AchievementText / sizeof(char*)));

	addToTracking(Achievement::FINISH_TUTORIAL);
	addToTracking(Achievement::BEAT_LEVEL_ALL_ALIVE);
	addToTracking(Achievement::BEAT_BOSS_LOW_HP);
	addToTracking(Achievement::BEAT_RECIPE_ALL_ALIVE);
	addToTracking(Achievement::BEAT_RECIPE);

	finishedTutorialListener = EventSystem<FinishedTutorialEvent>::instance().registerListener(
		std::bind(&AchievementSystem::onFinishedTutorialEvent, this));
	reachedBossListener = EventSystem<ReachedBossEvent>::instance().registerListener(
		std::bind(&AchievementSystem::onReachedBossEvent, this));
	beatLevelListener = EventSystem<BeatLevelEvent>::instance().registerListener(
		std::bind(&AchievementSystem::onBeatLevelEvent, this));
	defeatedBossListener = EventSystem<DefeatedBossEvent>::instance().registerListener(
		std::bind(&AchievementSystem::onDefeatedBossEvent, this));
}

AchievementSystem::~AchievementSystem()
{
	if (finishedTutorialListener.isValid())
	{
		EventSystem<FinishedTutorialEvent>::instance().unregisterListener(finishedTutorialListener);
	}
	if (reachedBossListener.isValid())
	{
		EventSystem<ReachedBossEvent>::instance().unregisterListener(reachedBossListener);
	}
	if (beatLevelListener.isValid())
	{
		EventSystem<BeatLevelEvent>::instance().unregisterListener(beatLevelListener);
	}
	if (defeatedBossListener.isValid())
	{
		EventSystem<DefeatedBossEvent>::instance().unregisterListener(defeatedBossListener);
	}
}

void AchievementSystem::onReachedBossEvent()
{
	// TODO: Achievements that happen multiple times for each recipe?
	// addToTracking(Achievement::BEAT_RECIPE);
}

void AchievementSystem::onBeatLevelEvent()
{
	size_t numPlayers = ECS::registry<PlayerComponent>.entities.size();
	if (numPlayers == 4 && isTracking(Achievement::BEAT_LEVEL_ALL_ALIVE))
	{
		addAchievement(Achievement::BEAT_LEVEL_ALL_ALIVE);
	}
}

void AchievementSystem::onDefeatedBossEvent()
{
	// TODO: Should change achievement names with multiple playthroughs/recipes
	if (isTracking(Achievement::BEAT_RECIPE))
	{
		addAchievement(Achievement::BEAT_RECIPE);
	}

	bool isLowHP = true;
	auto& players = ECS::registry<PlayerComponent>.entities;
	// If all players have HP < 15% of their max HP, then give achievement
	if (isTracking(Achievement::BEAT_BOSS_LOW_HP))
	{
		for (auto& player : players)
		{
			auto& playerStats = player.get<StatsComponent>();
			if (playerStats.getStatValue(StatType::HP) < 0.15 * playerStats.getStatValue(StatType::MAX_HP))
			{
				addAchievement(Achievement::BEAT_BOSS_LOW_HP);
			}
		}
	}
	// If all players are alive, give achievement
	if (isTracking(Achievement::BEAT_RECIPE_ALL_ALIVE))
	{
		if (players.size() == 4)
		{
			addAchievement(Achievement::BEAT_RECIPE_ALL_ALIVE);
		}
	}
}

void AchievementSystem::createMessage(vec2 position, Achievement achievement)
{
	ECS::Entity achievementEntity = createAchievementText(std::string(AchievementText[achievement]), position);

	std::string key = "message_box";
	ShadedMesh& resource = cacheResource(key);
	if (resource.effect.program.resource == 0) {
		// create a procedural circle
		constexpr float z = -0.1f;
		vec3 red = { 0.8,0.1,0.1 };

		// Corner points
		ColoredVertex v;
		v.position = { -0.5,-0.5,z };
		v.color = red;
		resource.mesh.vertices.push_back(v);
		v.position = { -0.5,0.5,z };
		v.color = red;
		resource.mesh.vertices.push_back(v);
		v.position = { 0.5,0.5,z };
		v.color = red;
		resource.mesh.vertices.push_back(v);
		v.position = { 0.5,-0.5,z };
		v.color = red;
		resource.mesh.vertices.push_back(v);

		// Two triangles
		resource.mesh.vertex_indices.push_back(0);
		resource.mesh.vertex_indices.push_back(1);
		resource.mesh.vertex_indices.push_back(3);
		resource.mesh.vertex_indices.push_back(1);
		resource.mesh.vertex_indices.push_back(2);
		resource.mesh.vertex_indices.push_back(3);
		resource.mesh.vertex_indices.push_back(0);
		resource.mesh.vertex_indices.push_back(2);
		resource.mesh.vertex_indices.push_back(3);

		RenderSystem::createColoredMesh(resource, "colored_mesh");
	}

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	ECS::registry<ShadedMeshRef>.emplace(achievementEntity, resource);
	achievementEntity.emplace<RenderableComponent>(RenderLayer::UI_TOOLTIP);

	// Create motion
	auto& motion = ECS::registry<Motion>.emplace(achievementEntity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = { 100.f, 200.f };


	achievementEntity.emplace<AchievementPopup>();
	achievementEntity.emplace<TimerComponent>(2500.0);
}

void AchievementSystem::onFinishedTutorialEvent()
{
	if (isTracking(Achievement::FINISH_TUTORIAL))
	{
		addAchievement(Achievement::FINISH_TUTORIAL);
	}
}

// Helper function
bool AchievementSystem::isTracking(Achievement item)
{
	for (auto curr : tracking)
	{
		if (curr == item)
			return true;
	}
	return false;
}

void AchievementSystem::addToTracking(Achievement item)
{
	tracking.push_back(item);
}

void AchievementSystem::removeFromTracking(Achievement item)
{
	tracking.remove(item);
}
