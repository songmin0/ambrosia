#include "achievement_system.hpp"
#include "stats_component.hpp"

AchievementSystem::AchievementSystem()
{
	//assert(sizeof(Achievement::SIZE_OF_ENUM) == sizeof(AchievementText / sizeof(char*)));

	addToTracking(Achievement::FINISH_TUTORIAL);
	addToTracking(Achievement::BEAT_LEVEL_ALL_ALIVE);
	addToTracking(Achievement::BEAT_BOSS_LOW_HP);
	addToTracking(Achievement::BEAT_RECIPE_ALL_ALIVE);

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

void AchievementSystem::addToTracking(Achievement item)
{
	tracking.push_back(item);
}

void AchievementSystem::removeFromTracking(Achievement item)
{
	tracking.remove(item);
}

void AchievementSystem::onReachedBossEvent()
{
	// TODO: Achievements that happen multiple times for each recipe?
	addToTracking(Achievement::BEAT_RECIPE);
}

void AchievementSystem::onBeatLevelEvent()
{
	size_t numPlayers = ECS::registry<PlayerComponent>.entities.size();
	if (numPlayers == 4 && isTracking(Achievement::BEAT_LEVEL_ALL_ALIVE))
	{
		addAchievement(Achievement::BEAT_LEVEL_ALL_ALIVE);
		removeFromTracking(Achievement::BEAT_LEVEL_ALL_ALIVE);
	}
}

void AchievementSystem::onDefeatedBossEvent()
{
	// TODO: Should change achievement names with multiple playthroughs/recipes
	addAchievement(Achievement::BEAT_RECIPE);
	removeFromTracking(Achievement::BEAT_RECIPE);

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
				removeFromTracking(Achievement::BEAT_BOSS_LOW_HP);
			}
		}
	}
	// If all players are alive, give achievement
	if (isTracking(Achievement::BEAT_RECIPE_ALL_ALIVE))
	{
		if (players.size() == 4)
		{
			addAchievement(Achievement::BEAT_RECIPE_ALL_ALIVE);
			removeFromTracking(Achievement::BEAT_RECIPE_ALL_ALIVE);
		}
	}
}

void AchievementSystem::onFinishedTutorialEvent()
{
	if (isTracking(Achievement::FINISH_TUTORIAL))
	{
		addAchievement(Achievement::FINISH_TUTORIAL);
		removeFromTracking(Achievement::FINISH_TUTORIAL);
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
