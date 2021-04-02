#include "achievement_system.hpp"
#include "stats_component.hpp"

AchievementSystem::AchievementSystem()
{
	//assert(sizeof(Achievement::SIZE_OF_ENUM) == sizeof(AchievementText / sizeof(char*)));

	tracking.push_back(Achievement::FINISH_TUTORIAL);
	tracking.push_back(Achievement::BEAT_LEVEL_ALL_ALIVE);
	tracking.push_back(Achievement::BEAT_RECIPE_ALL_ALIVE);

	finishedTutorialListener = EventSystem<FinishedTutorialEvent>::instance().registerListener(
		std::bind(&AchievementSystem::onFinishedTutorialEvent, this));
	reachedBossListener = EventSystem<ReachedBossEvent>::instance().registerListener(
		std::bind(&AchievementSystem::onReachedBossEvent, this));
	defeatedBossListener = EventSystem<DefeatedBossEvent>::instance().registerListener(
		std::bind(&AchievementSystem::onDefeatedBossEvent, this, std::placeholders::_1));
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
	addToTracking(Achievement::BEAT_RECIPE);
	addToTracking(Achievement::BEAT_BOSS_LOW_HP);
}

void AchievementSystem::onDefeatedBossEvent(const DefeatedBossEvent& event)
{
	// TODO: Should change achievement names with multiple playthroughs/recipes
	// Don't remove from tracking since we will have multiple recipes/bosses
	addAchievement(Achievement::BEAT_RECIPE);

	bool isLowHP = true;
	auto& players = ECS::registry<PlayerComponent>.entities;
	for (auto& player : players)
	{
		auto& playerStats = player.get<StatsComponent>();
		// If even one player has HP > 15% of their max HP, then no achievement for low HP
		if (playerStats.getStatValue(StatType::HP) > 0.15 * playerStats.getStatValue(StatType::MAX_HP))
		{
			return;
		}
	}
	addAchievement(Achievement::BEAT_BOSS_LOW_HP);
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
