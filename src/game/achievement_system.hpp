#pragma once
#include "common.hpp"
#include "event_system.hpp"
#include "events.hpp"

#include "entities/tiny_ecs.hpp"

enum Achievement {
	FINISH_TUTORIAL,
	BEAT_LEVEL_ALL_ALIVE,
	BEAT_RECIPE,
	BEAT_RECIPE_ALL_ALIVE,
	BEAT_BOSS_LOW_HP,
	SIZE_OF_ENUM
};
static const char* AchievementText[] = { "Finished the tutorial!", "Beat a level without anyone dying!", "Beat a recipe!", 
										 "Beat a recipe without anyone dying!", "Defeated a boss with low HP" };

struct AchievementPopup {};


class AchievementSystem {
protected:
	std::list<Achievement> achievements;
	std::list<Achievement> tracking;

	AchievementSystem();
	~AchievementSystem();

public:
	// Returns the singleton instance of this system
	static AchievementSystem& instance() {
		static AchievementSystem achievementSystem;
		return achievementSystem;
	}

	void addAchievement(Achievement item) 
	{ 
		achievements.push_back(item);
		createMessage({ 100.f, 150.f }, item);
		removeFromTracking(item);
	};
	void clearAchievements() { achievements.clear(); }
	std::list<Achievement> getAchievements() { return achievements; };

private:
	EventListenerInfo finishedTutorialListener;
	EventListenerInfo reachedBossListener;
	EventListenerInfo beatLevelListener;
	EventListenerInfo defeatedBossListener;

	void onFinishedTutorialEvent();
	void onReachedBossEvent();
	void onBeatLevelEvent();
	void onDefeatedBossEvent();

	void createMessage(vec2 position, Achievement achievement);
	bool isTracking(Achievement item);
	void addToTracking(Achievement item);
	void removeFromTracking(Achievement item);
};